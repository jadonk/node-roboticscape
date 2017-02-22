#include <nan.h>
#include <functional>

extern "C" {
    #include <stdio.h>
    #include <string.h>
    #include <uv.h>
    #include <roboticscape.h>
}

typedef void (*void_fp)();

/*
template<typename TCallable>
struct Wrapper
{
    typedef Wrapper<TCallable> Self;

    TCallable function;

    Wrapper(TCallable const & function)
    : function(function)
    {
        // Nothing else
    }

    static void call()
    {
        Self * wrapper = reinterpret_cast<Self * >();
        wrapper->function();
    }
};

typedef Wrapper<std::function<void()>> MyWrapper;
*/

// OT => Object Type
// RT => Return Type
// A ... => Arguments
template<typename OT, typename RT, typename ... A>
struct lambda_expression {
    OT _object;
    RT(OT::*_function)(A...)const;

    lambda_expression(const OT & object)
        : _object(object), _function(&decltype(_object)::operator()) {}

    RT operator() (A ... args) const {
        return (_object.*_function)(args...);
    }
};

namespace rc {
    void RCinitialize(const Nan::FunctionCallbackInfo<v8::Value>& info) {
        v8::Local<v8::Boolean> i = Nan::New((bool)rc_initialize());
        info.GetReturnValue().Set(i);
    }
    
    static void RCexit(void*) {
        rc_cleanup();
    }

    void RCgetState(const Nan::FunctionCallbackInfo<v8::Value>& info) {
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
    }

    void RCsetState(const Nan::FunctionCallbackInfo<v8::Value>& info) {
        if (info.Length() != 1) {
            Nan::ThrowTypeError("Wrong number of arguments (should be 1)");
            return;
        }
        if (!info[0]->IsString()) {
            Nan::ThrowTypeError("Wrong type (should be string)");
            return;
        }
        v8::String::Utf8Value str(info[0]->ToString());
        char * s = (char *)*str;
        if(!strcmp(s, "RUNNING")) rc_set_state(RUNNING);
        else if(!strcmp(s, "PAUSED")) rc_set_state(PAUSED);
        else if(!strcmp(s, "EXITING")) rc_set_state(EXITING);
        else if(!strcmp(s, "UNINITIALIZED")) rc_set_state(UNINITIALIZED);
        else {
            Nan::ThrowTypeError("Wrong value (should be 'RUNNING', "\
                    "'PAUSED', 'EXITING', or 'UNINITIALIZED'");
            return;
        }
    }
    
    struct Handoff {
        Nan::Callback cb;
    };
    
    static void doHandoff(uv_async_t* handle) {
        Nan::HandleScope scope;
        Handoff *h = static_cast<Handoff *>(handle->data);
        h->cb.Call(0, 0);
    }

    Handoff handoffPausePressed;
    uv_async_t pausePressedSync;
    
    void handoffPausePressedSync() {
        uv_async_send(&pausePressedSync);
    }

    void RCsetPausePressed(const Nan::FunctionCallbackInfo<v8::Value>& info) {
        if (info.Length() != 1) {
            Nan::ThrowTypeError("Wrong number of arguments (should be 1)");
            return;
        }
        if (!info[0]->IsFunction()) {
            Nan::ThrowTypeError("Wrong type (should be function)");
            return;
        }
        v8::Local<v8::Function> fn = info[0].As<v8::Function>();
        handoffPausePressed.cb.SetFunction(fn);
        pausePressedSync.data = &handoffPausePressed;
        uv_loop_t *loop = uv_default_loop();
        uv_async_init(loop, &pausePressedSync, doHandoff);
        rc_set_pause_pressed_func(handoffPausePressedSync);
    }

    Handoff handoffPauseReleased;
    uv_async_t pauseReleasedSync;
    
    void handoffPauseReleasedSync() {
        uv_async_send(&pauseReleasedSync);
    }

    void RCsetPauseReleased(const Nan::FunctionCallbackInfo<v8::Value>& info) {
        if (info.Length() != 1) {
            Nan::ThrowTypeError("Wrong number of arguments (should be 1)");
            return;
        }
        if (!info[0]->IsFunction()) {
            Nan::ThrowTypeError("Wrong type (should be function)");
            return;
        }
        v8::Local<v8::Function> fn = info[0].As<v8::Function>();
        handoffPauseReleased.cb.SetFunction(fn);
        pauseReleasedSync.data = &handoffPauseReleased;
        uv_loop_t *loop = uv_default_loop();
        uv_async_init(loop, &pauseReleasedSync, doHandoff);
        rc_set_pause_released_func(handoffPauseReleasedSync);
    }

    Handoff handoffModePressed;
    uv_async_t modePressedSync;
    
    void handoffModePressedSync() {
        uv_async_send(&modePressedSync);
    }

    void RCsetModePressed(const Nan::FunctionCallbackInfo<v8::Value>& info) {
        if (info.Length() != 1) {
            Nan::ThrowTypeError("Wrong number of arguments (should be 1)");
            return;
        }
        if (!info[0]->IsFunction()) {
            Nan::ThrowTypeError("Wrong type (should be function)");
            return;
        }
        v8::Local<v8::Function> fn = info[0].As<v8::Function>();
        handoffModePressed.cb.SetFunction(fn);
        modePressedSync.data = &handoffModePressed;
        uv_loop_t *loop = uv_default_loop();
        uv_async_init(loop, &modePressedSync, doHandoff);
        rc_set_mode_pressed_func(handoffModePressedSync);
    }

    Handoff handoffModeReleased;
    uv_async_t modeReleasedSync;
    
    void handoffModeReleasedSync() {
        uv_async_send(&modeReleasedSync);
    }

    void RCsetModeReleased(const Nan::FunctionCallbackInfo<v8::Value>& info) {
        if (info.Length() != 1) {
            Nan::ThrowTypeError("Wrong number of arguments (should be 1)");
            return;
        }
        if (!info[0]->IsFunction()) {
            Nan::ThrowTypeError("Wrong type (should be function)");
            return;
        }
        v8::Local<v8::Function> fn = info[0].As<v8::Function>();
        handoffModeReleased.cb.SetFunction(fn);
        pauseReleasedSync.data = &handoffModeReleased;
        uv_loop_t *loop = uv_default_loop();
        uv_async_init(loop, &modeReleasedSync, doHandoff);
        rc_set_mode_released_func(handoffModeReleasedSync);
    }

    void ModuleInit(v8::Local<v8::Object> exports) {
        exports->Set(Nan::New("initialize").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(RCinitialize)->GetFunction());
        exports->Set(Nan::New("get_state").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(RCgetState)->GetFunction());
        exports->Set(Nan::New("set_state").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(RCsetState)->GetFunction());
        exports->Set(Nan::New("set_pause_pressed_func").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(RCsetPausePressed)->GetFunction());
        exports->Set(Nan::New("set_pause_released_func").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(RCsetPauseReleased)->GetFunction());
        exports->Set(Nan::New("set_mode_pressed_func").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(RCsetModePressed)->GetFunction());
        exports->Set(Nan::New("set_mode_released_func").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(RCsetModeReleased)->GetFunction());
        node::AtExit(RCexit);
    }

    NODE_MODULE(roboticscape, ModuleInit);
}   // namespace rc