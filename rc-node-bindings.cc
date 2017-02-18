#include <node.h>

extern "C" {
    #include <stdio.h>
    #include <string.h>
    #include <roboticscape.h>
}

namespace rc {
    void RCinitialize(const v8::FunctionCallbackInfo<v8::Value>& args) {
        v8::Isolate* isolate = args.GetIsolate();
        bool i = (bool)rc_initialize();
        args.GetReturnValue().Set(v8::Boolean::New(isolate, i));
    }
    
    static void RCexit(void*) {
        rc_cleanup();
    }

    void RCgetState(const v8::FunctionCallbackInfo<v8::Value>& args) {
        v8::Isolate* isolate = args.GetIsolate();
        rc_state_t s = rc_get_state();
        switch(s) {
	    case RUNNING:
            args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, "RUNNING"));
            break;
	    case PAUSED:
            args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, "PAUSED"));
            break;
	    case EXITING:
            args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, "EXITING"));
            break;
        case UNINITIALIZED:
        default:
            args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, "UNINITIALIZED"));
            break;
        }
    }

    void RCsetState(const v8::FunctionCallbackInfo<v8::Value>& args) {
        v8::Isolate* isolate = args.GetIsolate();
        if (args.Length() != 1) {
            isolate->ThrowException(v8::Exception::TypeError(
                v8::String::NewFromUtf8(isolate, 
                    "must be only 1 argument")));
            return;
        }
        if (!args[0]->IsString()) {
            isolate->ThrowException(v8::Exception::TypeError(
                v8::String::NewFromUtf8(isolate,
                    "argument must be a string")));
            return;
        }
        v8::String::Utf8Value str(args[0]->ToString());
        char * s = (char *)*str;
        if(!strcmp(s, "RUNNING")) rc_set_state(RUNNING);
        else if(!strcmp(s, "PAUSED")) rc_set_state(PAUSED);
        else if(!strcmp(s, "EXITING")) rc_set_state(EXITING);
        else if(!strcmp(s, "UNINITIALIZED")) rc_set_state(UNINITIALIZED);
        else {
            isolate->ThrowException(v8::Exception::TypeError(
                v8::String::NewFromUtf8(isolate,
                    "argument must be a one of 'RUNNING', "\
                    "'PAUSED', 'EXITING', or 'UNINITIALIZED'")));
            return;
        }
    }
        
    void module_init(v8::Local<v8::Object> exports) {
        NODE_SET_METHOD(exports, "initialize", RCinitialize);
        NODE_SET_METHOD(exports, "get_state", RCgetState);
        NODE_SET_METHOD(exports, "set_state", RCsetState);
        node::AtExit(RCexit);
    }

    NODE_MODULE(roboticscape, module_init);
}   // namespace rc