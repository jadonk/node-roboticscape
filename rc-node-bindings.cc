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
    
    void module_init(v8::Local<v8::Object> exports) {
        NODE_SET_METHOD(exports, "initialize", RCinitialize);
        node::AtExit(RCexit);
    }

    NODE_MODULE(roboticscape, module_init);
}   // namespace rc