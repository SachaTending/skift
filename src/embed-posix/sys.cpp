#include <embed/sys.h>
#include <fcntl.h>
#include <karm-io/funcs.h>
#include <karm-sys/chan.h>
#include <sys/mman.h>
#include <unistd.h>

#include "utils.h"

namespace Embed {

struct PosixFd : public Sys::Fd {
    int _raw;

    PosixFd(int raw) : _raw(raw) {}

    Result<size_t> read(void *buf, size_t size) override {
        ssize_t result = ::read(_raw, buf, size);

        if (result < 0) {
            return Posix::fromLastErrno();
        }

        return result;
    }

    Result<size_t> write(void const *buf, size_t size) override {
        ssize_t result = ::write(_raw, buf, size);

        if (result < 0) {
            return Posix::fromLastErrno();
        }

        return result;
    }

    Result<size_t> seek(Io::Seek seek) override {
        off_t offset = 0;

        switch (seek.whence) {
        case Io::Whence::BEGIN:
            offset = 0;
            break;
        case Io::Whence::CURRENT:
            offset = lseek(_raw, seek.offset, SEEK_CUR);
            break;
        case Io::Whence::END:
            offset = lseek(_raw, seek.offset, SEEK_END);
            break;
        }

        if (offset < 0) {
            return Posix::fromLastErrno();
        }

        return offset;
    }

    Result<size_t> flush() override {
        // NOTE: No-op
        return 0;
    }

    Result<Strong<Fd>> dup() override {
        int duped = ::dup(_raw);

        if (duped < 0) {
            return Posix::fromLastErrno();
        }

        return {makeStrong<PosixFd>(duped)};
    }
};

Result<Strong<Sys::Fd>> openFile(Sys::Path path) {
    String str = path.str();
    int fd = ::open(str.buf(), O_RDONLY);

    if (fd < 0) {
        return Posix::fromLastErrno();
    }

    return {makeStrong<PosixFd>(fd)};
}

Result<Strong<Sys::Fd>> createFile(Sys::Path path) {
    String str = path.str();

    int fd = ::open(str.buf(), O_RDWR | O_CREAT, 0644);

    if (fd < 0) {
        return Posix::fromLastErrno();
    }

    return {makeStrong<PosixFd>(fd)};
}

Result<Pair<Strong<Sys::Fd>>> createPipe() {
    int fds[2];

    if (::pipe(fds) < 0) {
        return Posix::fromLastErrno();
    }

    return Pair<Strong<Sys::Fd>>{
        makeStrong<PosixFd>(fds[0]),
        makeStrong<PosixFd>(fds[1]),
    };
}

Result<Strong<Sys::Fd>> createIn() {
    return {makeStrong<PosixFd>(0)};
}

Result<Strong<Sys::Fd>> createOut() {
    return {makeStrong<PosixFd>(1)};
}

Result<Strong<Sys::Fd>> createErr() {
    return {makeStrong<PosixFd>(2)};
}

int mmapOptionsToProt(Karm::Sys::MmapOptions const &options) {
    int prot = 0;

    if (options.flags & Karm::Sys::MmapFlags::READ)
        prot |= PROT_READ;

    if (options.flags & Karm::Sys::MmapFlags::WRITE)
        prot |= PROT_WRITE;

    if (options.flags & Karm::Sys::MmapFlags::EXEC)
        prot |= PROT_EXEC;

    return prot;
}

Result<USizeRange> memMap(Karm::Sys::MmapOptions const &options) {
    void *addr = mmap((void *)options.vaddr, options.size, mmapOptionsToProt(options), MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if (addr == MAP_FAILED) {
        return Posix::fromLastErrno();
    }

    return USizeRange{(size_t)addr, options.size};
}

Result<USizeRange> memMap(Karm::Sys::MmapOptions const &options, Strong<Sys::Fd> maybeFd) {
    Strong<PosixFd> fd = try$(maybeFd.as<PosixFd>());
    size_t size = options.size;

    if (size == 0) {
        size = Io::size(*fd);
        Sys::println("File size {}", size);
    }

    void *addr = mmap((void *)options.vaddr, options.size, mmapOptionsToProt(options), MAP_SHARED, fd->_raw, options.offset);

    if (addr == MAP_FAILED) {
        return Posix::fromLastErrno();
    }

    return USizeRange{(size_t)addr, options.size};
}

Error memUnmap(void const *buf, size_t len) {
    if (::munmap((void *)buf, len) < 0) {
        return Posix::fromLastErrno();
    }

    return OK;
}

Error memFlush(void *flush, size_t len) {
    if (::msync(flush, len, MS_INVALIDATE) < 0) {
        return Posix::fromLastErrno();
    }

    return OK;
}

} // namespace Embed