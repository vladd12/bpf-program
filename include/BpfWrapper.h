#pragma once

#include <bcc/BPF.h>

class BpfWrapper {
private:
  std::unique_ptr<ebpf::BPF> bpfPtr;

public:
  int b;
};
