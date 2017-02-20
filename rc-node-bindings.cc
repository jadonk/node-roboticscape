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
        Handoff() {
        };
        Handoff(const Handoff& other) {
            std::swap(async, (uv_async_t &)other.async);
            cb.SetFunction(other.cb.GetFunction());
        };
        Handoff& operator=(Handoff other)
        {
            std::swap(async, other.async);
            cb.SetFunction(other.cb.GetFunction());
            return *this;
        }
        Handoff& operator=(const Handoff &other)
        {
            std::swap(async, (uv_async_t &)other.async);
            cb.SetFunction(other.cb.GetFunction());
            return *this;
        }
        uv_async_t async;
        Nan::Callback cb;
        void handler() {
            uv_async_send(&async);
        }

        /*
        auto lambdaHandler() {
            auto lambda = [=]() {
                handler();
            };
            return lambda_expression<decltype(lambda), void>(lambda);
        }
        void_fp getHandler() {
            return (void_fp) std::mem_fn(&Handoff::handler);
        }
        void_fp getHandler() {
            std::function<void()>::func callback = std::bind(method, this);
            void (*c_function_pointer)() = 
                static_cast<decltype(c_function_pointer)>(Callback<void()>::callback);
            fprintf(stderr, "getHandler: %p, %p, %p\n",
                (void *)method, (void *)this, (void *)c_function_pointer);
            fflush(stderr);
            return c_function_pointer;
        }
        */
    };
    
    static void doHandoff(uv_async_t* handle) {
        Nan::HandleScope scope;
        fprintf(stderr, "doHandoff: %p\n", (void *)handle);
        fflush(stderr);
        Handoff *h = static_cast<Handoff *>(handle->data);
        h->cb.Call(0, 0);
    }

    Handoff handoffPausePressed;

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
        handoffPausePressed.async.data = &handoffPausePressed;
        handoffPausePressed.cb.SetFunction(fn);
        uv_loop_t *loop = uv_default_loop();
        uv_async_init(loop, &(handoffPausePressed.async), doHandoff);
        //auto fp = [&] { h->handler(); };
        std::function<void()> f_handler = std::bind(&Handoff::handler,
            handoffPausePressed);
        void_fp * fp = f_handler.target<void_fp>();
        fprintf(stderr, "RCsetPausePressed: %p, %p\n", 
            (void *)&handoffPausePressed, (void *)fp);
        fflush(stderr);
        rc_set_pause_pressed_func(*fp);
    }
    
    /*
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
        Handoff *h = new Handoff(fn);
        fprintf(stderr, "RCsetPauseReleased: %p\n", (void *)h);
        fflush(stderr);
        h->async.data = h;
        h->cb.SetFunction(fn);
        uv_loop_t *loop = uv_default_loop();
        uv_async_init(loop, &(h->async), doHandoff);
        std::function<void()> f_handler = std::bind(&Handoff::handler, h);
        void_fp * fp = f_handler.target<void_fp>();
        rc_set_pause_released_func(*fp);
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
        Handoff *h = new Handoff(fn);
        fprintf(stderr, "RCsetModePressed: %p\n", (void *)h);
        fflush(stderr);
        h->async.data = h;
        h->cb.SetFunction(fn);
        uv_loop_t *loop = uv_default_loop();
        uv_async_init(loop, &(h->async), doHandoff);
        std::function<void()> f_handler = std::bind(&Handoff::handler, h);
        void_fp * fp = f_handler.target<void_fp>();
        rc_set_mode_pressed_func(*fp);
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
        Handoff *h = new Handoff(fn);
        fprintf(stderr, "RCsetModeReleased: %p\n", (void *)h);
        fflush(stderr);
        h->async.data = h;
        h->cb.SetFunction(fn);
        uv_loop_t *loop = uv_default_loop();
        uv_async_init(loop, &(h->async), doHandoff);
        std::function<void()> f_handler = std::bind(&Handoff::handler, h);
        void_fp * fp = f_handler.target<void_fp>();
        rc_set_mode_released_func(*fp);
    }
    */

    void ModuleInit(v8::Local<v8::Object> exports) {
        exports->Set(Nan::New("initialize").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(RCinitialize)->GetFunction());
        exports->Set(Nan::New("get_state").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(RCgetState)->GetFunction());
        exports->Set(Nan::New("set_state").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(RCsetState)->GetFunction());
        exports->Set(Nan::New("set_pause_pressed_func").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(RCsetPausePressed)->GetFunction());
            /*
        exports->Set(Nan::New("set_pause_released_func").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(RCsetPauseReleased)->GetFunction());
        exports->Set(Nan::New("set_mode_pressed_func").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(RCsetModePressed)->GetFunction());
        exports->Set(Nan::New("set_mode_released_func").ToLocalChecked(),
            Nan::New<v8::FunctionTemplate>(RCsetModeReleased)->GetFunction());
            */
        node::AtExit(RCexit);
    }

    NODE_MODULE(roboticscape, ModuleInit);
}   // namespace rc