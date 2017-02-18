extern "C" {
    #include <roboticscape.h>
}
#include <node.h>

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
    
    void module_init(v8::Local<v8::Object> exports) {
        NODE_SET_METHOD(exports, "initialize", RCinitialize);
        NODE_SET_METHOD(exports, "get_state", RCgetState);
        node::AtExit(RCexit);
    }

    NODE_MODULE(roboticscape, module_init);
}   // namespace rc