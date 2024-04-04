#pragma once

#include <iec_core/iec/iec_parser.h>
#include <iec_core/iec/validator.h>
#include <iec_core/utils/fast_file.h>
#include <iec_core/utils/socket.h>
#include <iostream>

class BPFExecutor;

namespace engines
{

class BPFEngine
{
private:
    utils::Buffer buf;
    utils::FastFile output;
    utils::Socket sock;
    iec::IecParser parser;
    iec::Validator validator;

    explicit BPFEngine(const std::string &filename);

public:
    explicit BPFEngine() = delete;
    explicit BPFEngine(const BPFEngine &rhs) = delete;
    explicit BPFEngine(BPFEngine &&rhs) = delete;
    BPFEngine &operator=(const BPFEngine &rhs) = delete;
    BPFEngine &operator=(BPFEngine &&rhs) = delete;

    explicit BPFEngine(const utils::Socket &socket, const std::string &filename);
    explicit BPFEngine(const BPFExecutor &executor, const std::string &filename);
    ~BPFEngine();

    void run();

    static BPFEngine create(const std::string_view &a, const std::string_view &b, const std::string_view &c);
};

} // namespace engines
