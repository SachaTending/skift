#include <abi-sysv/abi.h>
#include <handover/hook.h>
#include <hjert-api/api.h>
#include <karm-main/base.h>

extern "C" void __entryPoint(usize ho) {
    Abi::SysV::init();

    Handover::Payload *payload = (Handover::Payload *)ho;

    Ctx ctx;
    char const *argv[] = {"service", nullptr};
    ctx.add<ArgsHook>(1, argv);
    ctx.add<HandoverHook>(payload);

    auto res = entryPoint(ctx);

    auto self = Hj::Task::self();

    if (not res) {
        (void)self.crash();
    }

    Abi::SysV::fini();
    (void)self.ret();

    while (true)
        asm volatile("pause");
}
