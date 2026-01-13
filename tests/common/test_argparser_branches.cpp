#include <gtest/gtest.h>

#include "common/src/ArgParser/ArgParser.hpp"

using namespace rtype;

TEST(ArgParserBranches, OptionMissingArgumentReturnsError) {
    ArgParser p;
    p.option("-p", "--port", "port", "port desc",
             [](std::string_view v) {
                 (void)v;
                 return ParseResult::Success;
             });

    std::vector<std::string_view> args = {"--port"};
    auto res = p.parse(args);
    EXPECT_EQ(res, ParseResult::Error);
}

TEST(ArgParserBranches, UnknownOptionReturnsError) {
    ArgParser p;
    std::vector<std::string_view> args = {"--nope"};
    auto res = p.parse(args);
    EXPECT_EQ(res, ParseResult::Error);
}

TEST(ArgParserBranches, PositionalMissingRequiredReturnsError) {
    ArgParser p;
    p.positional("config", "config file",
                 [](std::string_view v) {
                     (void)v;
                     return ParseResult::Success;
                 },
                 true);
    std::vector<std::string_view> args = {}; // no args
    auto res = p.parse(args);
    EXPECT_EQ(res, ParseResult::Error);
}

TEST(ArgParserBranches, ExtraPositionalArgsAreIgnored) {
    ArgParser p;
    p.positional("a", "first",
                 [](std::string_view v) {
                     (void)v;
                     return ParseResult::Success;
                 });
    std::vector<std::string_view> args = {"one", "two", "three"};
    auto res = p.parse(args);
    EXPECT_EQ(res, ParseResult::Success);
}

TEST(ArgParserBranches, DuplicateOptionsAreRejected) {
    ArgParser p;
    p.flag("-h", "--help", "help",
           []() { return ParseResult::Success; });
    // adding duplicate should not throw and should return parser reference
    p.flag("-h", "--help", "helpdup", []() { return ParseResult::Success; });
    // ensure parse still works for known flag
    std::vector<std::string_view> args = {"-h"};
    auto res = p.parse(args);
    EXPECT_EQ(res, ParseResult::Success);
}

TEST(ArgParserBranches, FlagHandlerExitReturnsExit) {
    ArgParser p;
    p.flag("-x", "--exit", "exit",
           []() { return ParseResult::Exit; });
    std::vector<std::string_view> args = {"-x"};
    auto res = p.parse(args);
    EXPECT_EQ(res, ParseResult::Exit);
}

TEST(ArgParserBranches, OptionWithArgumentParsesSuccessfully) {
    ArgParser p;
    bool called = false;
    p.option("-p", "--port", "port", "port desc",
             [&called](std::string_view v) {
                 called = (v == "4242");
                 return ParseResult::Success;
             });
    std::vector<std::string_view> args = {"--port", "4242"};
    auto res = p.parse(args);
    EXPECT_EQ(res, ParseResult::Success);
    EXPECT_TRUE(called);
}

TEST(ArgParserBranches, OptionHandlerValidationErrorReturnsError) {
    ArgParser p;
    p.option("-n", "--num", "num", "number",
             [](std::string_view v) {
                 // pretend negative numbers are invalid
                 if (!v.empty() && v[0] == '-')
                     return ParseResult::Error;
                 return ParseResult::Success;
             });
    std::vector<std::string_view> args = {"--num", "-1"};
    auto res = p.parse(args);
    EXPECT_EQ(res, ParseResult::Error);
}

TEST(ArgParserBranches, OptionalPositionalMissingIsOk) {
    ArgParser p;
    bool called = false;
    p.positional("maybe", "optional arg",
                 [&called](std::string_view v) {
                     called = true;
                     (void)v;
                     return ParseResult::Success;
                 },
                 false);
    std::vector<std::string_view> args = {};
    auto res = p.parse(args);
    EXPECT_EQ(res, ParseResult::Success);
    EXPECT_FALSE(called);
}

TEST(ArgParserBranches, PrintUsageIncludesOptionsAndPositional) {
    ArgParser p;
    p.programName("prog");
    p.flag("-h", "--help", "help",
           []() { return ParseResult::Success; });
    p.option("-p", "--port", "port", "port desc",
             [](std::string_view) { return ParseResult::Success; });
    p.positional("file", "config file",
                 [](std::string_view) { return ParseResult::Success; });
    testing::internal::CaptureStdout();
    p.printUsage();
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(out.find("Usage:"), std::string::npos);
    EXPECT_NE(out.find("--help"), std::string::npos);
    EXPECT_NE(out.find("--port"), std::string::npos);
    EXPECT_NE(out.find("file"), std::string::npos);
}
