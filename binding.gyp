{
    "targets": [
        {
            "target_name": "roboticscape",
            "sources": [ "rc-node-bindings.cc" ],
            "cflags": [ "-Wall", "-g" ],
            "libraries": [ "-lm", "-lrt", "-lpthread", "-lroboticscape" ]
        }
    ]
}
