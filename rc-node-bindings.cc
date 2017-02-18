#include <nan.h>
#include <functional>

extern "C" {
    #include <string.h>
    #include <uv.h>
    #include <roboticscape.h>
}

typedef void (*void_fp)();

template <typename T>
struct Callback;

template <typename Ret, typename... Params>
struct Callback<Ret(Params...)> {
    template <typename... Args>
    static Ret callback(Args... args) { return func(args...); }
    static std::function<Ret(Params...)> func;
};

// Initialize the static member.
template <typename Ret, typename... Params>
std::function<Ret(Params...)> Callback<Ret(Params...)>::func;


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
        uv_async_t async;
        v8::Persistent<v8::Function> cb;
        void handler() {
            uv_async_send(&async);
        }
        template<typename T>
        void_fp getHandler(void (T::*method)(), T* r) {
            Callback<void()>::func = std::bind(method, r);
            void (*c_function_pointer)() = 
                static_cast<decltype(c_function_pointer)>(Callback<void()>::callback);
            return c_function_pointer;
        }
    };
    
    static void doHandoff(uv_async_t* handle) {
        Handoff *h = static_cast<Handoff *>(handle->data);
        const unsigned argc = 0;
        v8::Local<v8::Value> argv[argc] = { };
        v8::Local<v8::Function> cb = v8::Local<v8::Function>::New(v8::Isolate::GetCurrent(), h->cb);
        Nan::MakeCallback(Nan::GetCurrentContext()->Global(), cb, argc, argv);
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
        Handoff *h = new Handoff;
        h->async.data = h;

        //h->cb = v8::Persistent<v8::Function>::New(v8::Isolate::GetCurrent(), 
        //    &(info[0].As<v8::Function>()));
        uv_loop_t *loop = uv_default_loop();
        uv_async_init(loop, &(h->async), doHandoff);
        void_fp fp = h->getHandler(&Handoff::handler, h);
        rc_set_pause_pressed_func(fp);
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
        node::AtExit(RCexit);
    }

    NODE_MODULE(roboticscape, ModuleInit);
}   // namespace rc