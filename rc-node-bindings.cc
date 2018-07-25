#include <nan.h>
#include <functional>

extern "C" {
    #include <stdio.h>
    #include <string.h>
    #include <uv.h>
    #include <roboticscape.h>
}

typedef void (*void_fp)();

namespace rc {
    void RCinitialize(const Nan::FunctionCallbackInfo<v8::Value>& info) {
        v8::Local<v8::Boolean> i = Nan::New((bool)rc_initialize());
        rc_disable_signal_handler();
        info.GetReturnValue().Set(i);
    }

    static void RCexit(void*) {
        fprintf(stderr, "info: running rc_cleanup()\n");
        rc_cleanup();
    }

    void RCstate(const Nan::FunctionCallbackInfo<v8::Value>& info) {
        if (info.Length() == 0) {
            rc_state_t s = rc_get_state();
            switch(s) {
    	    case RUNNING:
                info.GetReturnValue().Set(Nan::New("RUNNING").ToLocalChecked());
                break;
    	    case PAUSED:
                info.GetReturnValue().Set(Nan::New("PAUSED").ToLocalChecked());
                break;
    	    case EXITING:
                info.GetReturnValue().Set(Nan::New("EXITING").ToLocalChecked());
                break;
            case UNINITIALIZED:
            default:
                info.GetReturnValue().Set(Nan::New("UNINITIALIZED").ToLocalChecked());
                break;
            }
            return;
        }
        else if (info.Length() != 1) {
            Nan::ThrowTypeError("Wrong number of arguments (should be 0 or 1)");
            return;
        }
        if (!info[0]->IsString()) {
            Nan::ThrowTypeError("Wrong type (should be string)");
            return;
        }
        v8::String::Utf8Value str(info[0]->ToString());
        char * s = (char *)*str;
        if (!strcmp(s, "RUNNING")) rc_set_state(RUNNING);
        else if (!strcmp(s, "PAUSED")) rc_set_state(PAUSED);
        else if (!strcmp(s, "EXITING")) rc_set_state(EXITING);
        else if (!strcmp(s, "UNINITIALIZED")) rc_set_state(UNINITIALIZED);
        else {
            Nan::ThrowTypeError("Wrong value (should be 'RUNNING', "\
                    "'PAUSED', 'EXITING', or 'UNINITIALIZED'");
            return;
        }
    }

    void RCLED(const Nan::FunctionCallbackInfo<v8::Value>& info) {
        if (info.Length() != 2) {
            Nan::ThrowTypeError("Wrong number of arguments (should be 2)");
            return;
        }
        if (!info[0]->IsString()) {
            Nan::ThrowTypeError("Wrong type (should be string)");
            return;
        }
        if (!info[1]->IsBoolean() && !info[1]->IsNumber()) {
            Nan::ThrowTypeError("Wrong type (should be boolean or number)");
            return;
        }
        v8::String::Utf8Value str(info[0]->ToString());
        char * s = (char *)*str;
        bool i = (bool)info[1]->ToBoolean()->Value();
        if (!strcmp(s, "GREEN")) rc_led_set(RC_LED_GREEN, i ? 1 : 0);
        else if (!strcmp(s, "RED")) rc_led_set(RC_LED_RED, i ? 1 : 0);
        else {
            Nan::ThrowTypeError("Wrong value (should be 'GREEN', "\
                    "or 'RED'");
            return;
        }
    }

    struct Handoff {
        Nan::Callback *cb;
    };

    static void doHandoff(uv_async_t* handle) {
        Nan::HandleScope scope;
        Handoff *h = static_cast<Handoff *>(handle->data);
#ifdef DEBUG
        fprintf(stderr, "Handoff %p using callback object %p to call %p\n",
            (void *)h, (void *)h->cb, h->cb->GetFunction());
        fflush(stderr);
#endif
        h->cb->Call(0, 0);
    }

    uv_async_t pausePressedSync,
        pauseReleasedSync,
        modePressedSync,
        modeReleasedSync;
    Handoff handoffPausePressed,
        handoffPauseReleased,
        handoffModePressed,
        handoffModeReleased;

    void handoffPausePressedSync() {
#ifdef DEBUG
        fprintf(stderr, "%s (%p) sent event %p\n", __func__,
            (void *)&handoffPausePressedSync, (void *)&pausePressedSync);
        fflush(stderr);
#endif
        uv_async_send(&pausePressedSync);
    }

    void handoffPauseReleasedSync() {
#ifdef DEBUG
        fprintf(stderr, "%s (%p) sent event %p\n", __func__,
            (void *)&handoffPauseReleasedSync, (void *)&pauseReleasedSync);
        fflush(stderr);
#endif
        uv_async_send(&pauseReleasedSync);
    }

    void handoffModePressedSync() {
#ifdef DEBUG
        fprintf(stderr, "%s (%p) sent event %p\n", __func__,
            (void *)&handoffModePressedSync, (void *)&modePressedSync);
        fflush(stderr);
#endif
        uv_async_send(&modePressedSync);
    }

    void handoffModeReleasedSync() {
#ifdef DEBUG
        fprintf(stderr, "%s (%p) sent event %p\n", __func__,
            (void *)&handoffModeReleasedSync, (void *)&modeReleasedSync);
        fflush(stderr);
#endif
        uv_async_send(&modeReleasedSync);
    }

    void RCon(const Nan::FunctionCallbackInfo<v8::Value>& info) {
        if (info.Length() != 2) {
            Nan::ThrowTypeError("Wrong number of arguments (should be 2)");
            return;
        }
        if (!info[0]->IsString()) {
            Nan::ThrowTypeError("Wrong type for argument 0 (should be string)");
            return;
        }
        if (!info[1]->IsFunction()) {
            Nan::ThrowTypeError("Wrong type for argument 1 (should be function)");
            return;
        }
        v8::String::Utf8Value str(info[0]->ToString());
        char * s = (char *)*str;
        v8::Local<v8::Function> fn = info[1].As<v8::Function>();
        Handoff * h;
        uv_async_t * event;
        void_fp fp;
        int (* set)(void (*func)(void));
        if (!strcmp(s, "PAUSE_PRESSED")) {
            h = &handoffPausePressed;
            event = &pausePressedSync;
            fp = handoffPausePressedSync;
            set = rc_set_pause_pressed_func;
        } else if (!strcmp(s, "PAUSE_RELEASED")) {
            h = &handoffPauseReleased;
            event = &pauseReleasedSync;
            fp = handoffPauseReleasedSync;
            set = rc_set_pause_released_func;
        } else if (!strcmp(s, "MODE_PRESSED")) {
            h = &handoffModePressed;
            event = &modePressedSync;
            fp = handoffModePressedSync;
            set = rc_set_mode_pressed_func;
        } else if (!strcmp(s, "MODE_RELEASED")) {
            h = &handoffModeReleased;
            event = &modeReleasedSync;
            fp = handoffModeReleasedSync;
            set = rc_set_mode_released_func;
        } else {
            Nan::ThrowTypeError("Wrong value (should be "\
                "'PAUSE_PRESSED', 'PAUSE_RELEASED' "\
                "'MODE_PRESSED', or 'MODE_RELEASED')");
            return;
        }
        h->cb = new Nan::Callback(fn);
        event->data = h;
        uv_loop_t *loop = uv_default_loop();
        uv_async_init(loop, event, doHandoff);
        set(fp);
#ifdef DEBUG
        fprintf(stderr, "Registered event %p " \
            "with C callback %p, handoff %p, callback object %p " \
            "and C++ function %p using function %p\n",
            (void *)event,
            (void* )fp, (void *)h, h->cb,
            h->cb->GetFunction(), (void *)set);
        fflush(stderr);
#endif
    }

    void RCmotor(const Nan::FunctionCallbackInfo<v8::Value>& info) {
        if (info.Length() == 2) {
            if (!info[0]->IsInt32()) {
                Nan::ThrowTypeError("Wrong type for argument 0 (should be integer)");
                return;
            }
            int motor = (int)info[0]->ToInt32()->Value();
            if (motor < 1 || motor > 4) {
                Nan::ThrowTypeError("Wrong value for argument 0 (should be 1 - 4)");
                return;
            }
            if (info[1]->IsString()) {
                v8::String::Utf8Value str(info[1]->ToString());
                char * s = (char *)*str;
                if (!strcmp(s, "FREE_SPIN")) rc_motor_free_spin(motor);
                else if (!strcmp(s, "BRAKE")) rc_motor_brake(motor);
                else {
                    Nan::ThrowTypeError("Wrong value for argument 1 "\
                        "(should be 'FREE_SPIN', 'BRAKE' or a numeric duty)");
                }
                return;
            }
            float duty = (float)info[1]->ToNumber()->Value();
            rc_motor_set(motor, duty);
            return;
        }
        if (info.Length() != 1) {
            Nan::ThrowTypeError("Wrong number of arguments (should be 1 or 2)");
            return;
        }
        if (!info[0]->IsString() && !info[0]->IsNumber()) {
            Nan::ThrowTypeError("Wrong type for argument (should be string or number)");
            return;
        }
        if (info[0]->IsString()) {
            v8::String::Utf8Value str(info[0]->ToString());
            char * s = (char *)*str;
            if (!strcmp(s, "ENABLE")) rc_motor_init();
            else if (!strcmp(s, "DISABLE")) rc_motor_cleanup();
            else if (!strcmp(s, "FREE_SPIN")) rc_motor_free_spin(0);
            else if (!strcmp(s, "BRAKE")) rc_motor_brake(0);
            else {
                Nan::ThrowTypeError("Wrong value (should be "\
                    "'ENABLE', 'DISABLE' "\
                    "'FREE_SPIN', 'BRAKE' or a numeric duty)");
            }
            return;
        }
        float duty = (float)info[0]->ToNumber()->Value();
        rc_motor_set(0,duty);
    }

    void RCencoder(const Nan::FunctionCallbackInfo<v8::Value>& info) {
        if (info.Length() == 2) {
            if (!info[0]->IsInt32()) {
                Nan::ThrowTypeError("Wrong type for argument 0 (should be integer)");
                return;
            }
            if (!info[0]->IsInt32()) {
                Nan::ThrowTypeError("Wrong type for argument 1 (should be integer)");
                return;
            }
            int encoder = (int)info[0]->ToInt32()->Value();
            int value = (int)info[1]->ToInt32()->Value();
            if (encoder < 1 || encoder > 4) {
                Nan::ThrowTypeError("Wrong value for argument 0 (should be 1 - 4)");
                return;
            }
            rc_encoder_write(encoder, value);
            return;
        }
        if (info.Length() != 1) {
            Nan::ThrowTypeError("Wrong number of arguments (should be 1 or 2)");
            return;
        }
        if (!info[0]->IsInt32()) {
            Nan::ThrowTypeError("Wrong type for argument (should be integer)");
            return;
        }
        int encoder = (int)info[0]->ToInt32()->Value();
        int i = rc_encoder_read(encoder);
        info.GetReturnValue().Set(i);
    }

    void RCadc(const Nan::FunctionCallbackInfo<v8::Value>& info) {
        float value = 0.0;
        if (info.Length() != 1) {
            Nan::ThrowTypeError("Wrong number of arguments (should be 1)");
            return;
        }
        if (!info[0]->IsString() && !info[0]->IsInt32()) {
            Nan::ThrowTypeError("Wrong type for argument (should be string or integer)");
            return;
        }
        rc_adc_init();
        if (info[0]->IsString()) {
            v8::String::Utf8Value str(info[0]->ToString());
            char * s = (char *)*str;
            if (!strcmp(s, "BATTERY")) value = rc_adc_batt();
            else if (!strcmp(s, "DC_JACK")) value = rc_adc_dc_jack();
            else {
                Nan::ThrowTypeError("Wrong value (should be "\
                    "'BATTERY', 'DC_JACK' "\
                    "0, 1, 2 or 3)");
                return;
            }
            info.GetReturnValue().Set(value);
            return;
        }
        int adc = (int)info[0]->ToInt32()->Value();
        if (adc < 0 || adc > 3) {
            Nan::ThrowTypeError("Wrong value (should be "\
                "'BATTERY', 'DC_JACK' "\
                "0, 1, 2 or 3)");
            return;
        }
        value = rc_adc_read_volt(adc);
        info.GetReturnValue().Set(value);
    }

    void RCservo(const Nan::FunctionCallbackInfo<v8::Value>& info) {
        int result = 0;
        if (info.Length() == 1) {
            if (info[0]->IsString()) {
                v8::String::Utf8Value str(info[0]->ToString());
                char * s = (char *)*str;
                if (!strcmp(s, "ENABLE")) rc_servo_init();
                else if (!strcmp(s, "POWER_RAIL_ENABLE")) rc_servo_power_rail_en(1);
                else if (!strcmp(s, "POWER_RAIL_DISABLE")) rc_servo_power_rail_en(0);
                else if (!strcmp(s, "DISABLE")) rc_servo_cleanup();
                else {
                    Nan::ThrowTypeError("Wrong value (should be "\
                        "'ENABLE', 'DISABLE' "\
                        "'POWER_RAIL_ENABLE', 'POWER_RAIL_DISABLE' or a numeric channel no)");
                }
                return;
            }
            else {
              Nan::ThrowTypeError("Wrong type for argument (should be String)");
              return;
            }
        }
        else {
            if (!info[0]->IsInt32()) {
                Nan::ThrowTypeError("Wrong type for argument (should be integer)");
                return;
            }
            int channel = (int)info[0]->ToInt32()->Value();
            float value = (float)info[1]->ToNumber()->Value();
            if (channel < 0 || channel > 8) {
                Nan::ThrowTypeError("Wrong value for argument 1 (should be between 0 and 8)");
                return;
            }
            result = rc_servo_send_pulse_normalized(channel, value );
            info.GetReturnValue().Set(result);
      }
    }

    void RCbmp(const Nan::FunctionCallbackInfo<v8::Value>& info) {
        if (info.Length() != 1) {
            Nan::ThrowTypeError("Wrong number of arguments (should be 1)");
            return;
        }
        if (!info[0]->IsString()) {
            Nan::ThrowTypeError("Wrong type for argument (should be string or integer)");
            return;
        }
        else {

            rc_bmp_data_t bmp_data;
            v8::String::Utf8Value str(info[0]->ToString());
            char * s = (char *)*str;
            if (!strcmp(s, "ENABLE")) rc_bmp_init( BMP_OVERSAMPLE_1, BMP_FILTER_OFF);
            else if (!strcmp(s, "DISABLE")) rc_bmp_power_off();
            else if (!strcmp(s, "READ")) rc_bmp_read(&bmp_data);
            else {
                Nan::ThrowTypeError("Wrong value (should be "\
                    "'ENABLE', 'DISABLE' "\
                    " or 'READ' )");
                return;
            }
            v8::Local<v8::Object> bmpObject = Nan::New<v8::Object>();
            v8::Local<v8::String> TempProp = Nan::New("temp_c").ToLocalChecked();
            v8::Local<v8::String> AltProp = Nan::New("alt_m").ToLocalChecked();
            v8::Local<v8::String> PressureProp = Nan::New("pressure_pa").ToLocalChecked();
            v8::Local<v8::Value> TempValue = Nan::New(bmp_data.temp_c);
            v8::Local<v8::Value> AltValue = Nan::New(bmp_data.alt_m);
            v8::Local<v8::Value> PressureValue = Nan::New(bmp_data.pressure_pa);
            Nan::Set(bmpObject, TempProp, TempValue);
            Nan::Set(bmpObject, AltProp, AltValue);
            Nan::Set(bmpObject, PressureProp, PressureValue);
            info.GetReturnValue().Set(bmpObject);
        }
    }
    rc_mpu_data_t mpu_data;
    void RCimu(const Nan::FunctionCallbackInfo<v8::Value>& info) {
        if (info.Length() != 1) {
            Nan::ThrowTypeError("Wrong number of arguments (should be 1)");
            return;
        }
        if (!info[0]->IsString()) {
            Nan::ThrowTypeError("Wrong type for argument (should be string)");
            return;
        }
        else {
            rc_mpu_config_t conf = rc_mpu_default_config();
            conf.enable_magnetometer = 1;
            v8::String::Utf8Value str(info[0]->ToString());
            char * s = (char *)*str;
            if (!strcmp(s, "ENABLE")) rc_mpu_initialize(&mpu_data, conf);
            else if (!strcmp(s, "DISABLE")) rc_mpu_power_off();
            else if (!strcmp(s, "READ_ACCEL")) rc_mpu_read_accel(&mpu_data);
            else if (!strcmp(s, "READ_GYRO")) rc_mpu_read_gyro(&mpu_data);
            else if (!strcmp(s, "READ_MAG")) rc_mpu_read_mag(&mpu_data);
            else if (!strcmp(s, "READ_TEMP")) rc_mpu_read_temp(&mpu_data);
            else if (!strcmp(s, "READ_ALL") || !strcmp(s, "READ")){
                rc_mpu_read_accel(&mpu_data);
                rc_mpu_read_gyro(&mpu_data);
                rc_mpu_read_mag(&mpu_data);
                rc_mpu_read_temp(&mpu_data);
            }
            else {
                Nan::ThrowTypeError("Wrong value (should be "\
                    "'ENABLE', 'DISABLE' "\
                    "'READ_ACCEL' 'READ_GYRO' 'READ_TEMP' 'READ_MAG' or 'READ_ALL' )");
                return;
            }

            v8::Local<v8::Object> mpuObject = Nan::New<v8::Object>();
            v8::Local<v8::String> AccelProp = Nan::New("accel").ToLocalChecked();
            v8::Local<v8::String> GyroProp = Nan::New("gyro").ToLocalChecked();
            v8::Local<v8::String> MagProp = Nan::New("mag").ToLocalChecked();
            v8::Local<v8::String> TempProp = Nan::New("temp").ToLocalChecked();
            v8::Local<v8::Array> AccelValue = Nan::New<v8::Array>(3);
            Nan::Set(AccelValue, 0, Nan::New(mpu_data.accel[0]));
            Nan::Set(AccelValue, 1, Nan::New(mpu_data.accel[1]));
            Nan::Set(AccelValue, 2, Nan::New(mpu_data.accel[2]));
            v8::Local<v8::Array> GyroValue = Nan::New<v8::Array>(3);
            Nan::Set(GyroValue, 0, Nan::New(mpu_data.gyro[0]));
            Nan::Set(GyroValue, 1, Nan::New(mpu_data.gyro[1]));
            Nan::Set(GyroValue, 2, Nan::New(mpu_data.gyro[2]));
            v8::Local<v8::Array> MagValue = Nan::New<v8::Array>(3);
            Nan::Set(MagValue, 0, Nan::New(mpu_data.mag[0]));
            Nan::Set(MagValue, 1, Nan::New(mpu_data.mag[1]));
            Nan::Set(MagValue, 2, Nan::New(mpu_data.mag[2]));
            v8::Local<v8::Value> TempValue = Nan::New(mpu_data.temp);
            Nan::Set(mpuObject, AccelProp, AccelValue);
            Nan::Set(mpuObject, GyroProp, GyroValue);
            Nan::Set(mpuObject, MagProp, MagValue);
            Nan::Set(mpuObject, TempProp, TempValue);
            info.GetReturnValue().Set(mpuObject);
    }
  }

    void ModuleInit(v8::Local<v8::Object> exports) {
        /* Init and Cleanup */
        exports->Set(Nan::New("initialize").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(RCinitialize)->GetFunction());
        /* Flow State */
        exports->Set(Nan::New("state").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(RCstate)->GetFunction());
        /* LEDs */
        exports->Set(Nan::New("led").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(RCLED)->GetFunction());
        /* Buttons */
        exports->Set(Nan::New("on").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(RCon)->GetFunction());
        /* DC motors */
        exports->Set(Nan::New("motor").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(RCmotor)->GetFunction());
        /* encoders */
        exports->Set(Nan::New("encoder").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(RCencoder)->GetFunction());
        /* ADC */
        exports->Set(Nan::New("adc").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(RCadc)->GetFunction());
        /* Servo */
        exports->Set(Nan::New("servo").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(RCservo)->GetFunction());
        /* BMP */
        exports->Set(Nan::New("bmp").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(RCbmp)->GetFunction());
        /* IMU */
        exports->Set(Nan::New("imu").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(RCimu)->GetFunction());
        exports->Set(Nan::New("mpu").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(RCimu)->GetFunction());
        node::AtExit(RCexit);
    }

    NODE_MODULE(roboticscape, ModuleInit);
}   // namespace rc
