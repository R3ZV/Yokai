const std = @import("std");
const OptimizeMode = std.builtin.OptimizeMode;
const ResolvedTarget = std.Build.ResolvedTarget;

const CXX_FLAGS = .{
    "-std=c++23",
    "-pedantic",
    "-Werror",
    "-Wall",
    "-Wextra",
};

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    build_repl(b, target, optimize);
    build_daemon(b, target, optimize);
    build_tests(b, target, optimize);

    const check_opt = b.option(bool, "check_format", "Check if project is formatted correctly") orelse false;
    format_code(b, check_opt);
    // build_docs(b, target, optimize);
}

const repl_files = .{
    "repl/main.cpp",
    "common/connection.cpp",
};

fn build_repl(
    b: *std.Build,
    target: ResolvedTarget,
    optimize: OptimizeMode,
) void {
    const exe = b.addExecutable(.{
        .name = "yokai-repl",
        .target = target,
        .optimize = optimize,
        .use_llvm = false,
    });

    exe.linkLibCpp();

    exe.addCSourceFiles(.{
        .root = b.path("."),
        .files = &(repl_files),
        .flags = &CXX_FLAGS,
    });

    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run-repl", "Run the repl");
    run_step.dependOn(&run_cmd.step);
}

const daemon_files = .{
    "daemon/main.cpp",
    "daemon/database.cpp",
    "daemon/transaction.cpp",
    "daemon/list_database.cpp",
    "daemon/object.cpp",
    "common/connection.cpp",
};

fn build_daemon(
    b: *std.Build,
    target: ResolvedTarget,
    optimize: OptimizeMode,
) void {
    const exe = b.addExecutable(.{
        .name = "yokai-daemon",
        .target = target,
        .optimize = optimize,
        .use_llvm = false,
    });

    exe.linkLibCpp();

    exe.addCSourceFiles(.{
        .root = b.path("."),
        .files = &daemon_files,
        .flags = &CXX_FLAGS,
    });

    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run-daemon", "Run the daemon");
    run_step.dependOn(&run_cmd.step);
}

const test_files = .{
    "tests/main.cpp",
    "daemon/object.cpp",
    "daemon/database.cpp",
};

fn build_tests(
    b: *std.Build,
    target: ResolvedTarget,
    optimize: OptimizeMode,
) void {
    const unit_tests = b.addExecutable(.{
        .name = "tests",
        .target = target,
        .optimize = optimize,
        .use_llvm = false,
    });

    unit_tests.linkLibCpp();

    unit_tests.addCSourceFiles(.{
        .root = b.path("."),
        .files = &test_files,
        .flags = &CXX_FLAGS,
    });

    b.installArtifact(unit_tests);

    const test_cmd = b.addRunArtifact(unit_tests);
    test_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        test_cmd.addArgs(args);
    }

    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&test_cmd.step);
}

fn build_docs(
    b: *std.Build,
    target: ResolvedTarget,
    optimize: OptimizeMode,
) void {
    _ = b;
    _ = target;
    _ = optimize;
    // https://ziglang.org/learn/build-system/#system-tools
}

const header_files = .{
    "common/include/connection.h",
    "daemon/include/object.h",
    "daemon/include/database.h",
    "daemon/include/list_database.h",
    "daemon/include/transaction.h",
};

fn format_code(b: *std.Build, check: bool) void {
    const files = daemon_files ++ repl_files ++ test_files ++ header_files;

    if (!check) {
        const flags = .{"-i"};
        const args = flags ++ files;
        const tool_run = b.addSystemCommand(&.{"clang-format"});
        tool_run.addArgs(&(args));

        const format_step = b.step("format", "Run code formatting");
        format_step.dependOn(&tool_run.step);
    } else {
        const flags = .{ "--dry-run", "--Werror" };
        const args = flags ++ files;

        const tool_run = b.addSystemCommand(&.{"clang-format"});
        tool_run.addArgs(&(args));

        const format_step = b.step("format", "Run code formatting");
        format_step.dependOn(&tool_run.step);
    }
}
