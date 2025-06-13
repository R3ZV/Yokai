const std = @import("std");
const Compile = std.Build.Step.Compile;
const OptimizeMode = std.builtin.OptimizeMode;
const ResolvedTarget = std.Build.ResolvedTarget;

const CXX_FLAGS = .{
    "-std=c++23",
    "-pedantic",
    "-Werror",
    "-Wall",
    "-Wextra",
};

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const check_opt = b.option(bool, "check_format", "Check if project is formatted correctly") orelse false;
    format_code(b, check_opt);
    build_docs(b);

    const check_release = b.option(bool, "release", "Run release build") orelse false;
    if (check_release) {
        try build_release(b);
    } else {
        _ = build_daemon(b, target, optimize, false);
        build_tests(b, target, optimize);
    }
}

fn build_release(b: *std.Build) !void {
    const targets: []const std.Target.Query = &.{
        .{ .cpu_arch = .aarch64, .os_tag = .linux },
        .{ .cpu_arch = .x86_64, .os_tag = .linux, .abi = .gnu },
        .{ .cpu_arch = .x86_64, .os_tag = .linux, .abi = .musl },
    };

    for (targets) |t| {
        const daemon_exe = build_daemon(b, b.resolveTargetQuery(t), .ReleaseSafe, true);

        const target_output_daemon = b.addInstallArtifact(daemon_exe, .{
            .dest_dir = .{
                .override = .{
                    .custom = try t.zigTriple(b.allocator),
                },
            },
        });

        b.getInstallStep().dependOn(&target_output_daemon.step);
    }
}

const daemon_files = .{
    "daemon/main.cpp",
    "daemon/database.cpp",
    "daemon/transaction.cpp",
    "daemon/list_database.cpp",
    "daemon/object.cpp",
    "daemon/command.cpp",
    "common/connection.cpp",
};

fn build_daemon(
    b: *std.Build,
    target: ResolvedTarget,
    optimize: OptimizeMode,
    release: bool,
) *Compile {
    const exe = b.addExecutable(.{
        .name = "yokai-daemon",
        .target = target,
        .optimize = optimize,
    });

    exe.linkLibCpp();

    exe.addCSourceFiles(.{
        .root = b.path("."),
        .files = &daemon_files,
        .flags = &CXX_FLAGS,
    });


    if (!release) {
        b.installArtifact(exe);
        const run_cmd = b.addRunArtifact(exe);
        run_cmd.step.dependOn(b.getInstallStep());
        if (b.args) |args| {
            run_cmd.addArgs(args);
        }

        const run_step = b.step("run-daemon", "Run the daemon");
        run_step.dependOn(&run_cmd.step);
    }
    return exe;
}

const test_files = .{
    "tests/unit/main.cpp",
    "tests/unit/test_database.cpp",
    "tests/unit/test_transaction.cpp",
    "tests/unit/test_command.cpp",

    "daemon/command.cpp",
    "daemon/database.cpp",
    "daemon/transaction.cpp",
    "daemon/list_database.cpp",
    "daemon/object.cpp",
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
) void {
    const args = .{"docs_config"};

    const tool_run = b.addSystemCommand(&.{"doxygen"});
    tool_run.addArgs(&(args));

    const docs_step = b.step("docs", "Generate docs");
    docs_step.dependOn(&tool_run.step);
}

const header_files = .{
    "common/include/connection.h",
    "daemon/include/object.h",
    "daemon/include/command.h",
    "daemon/include/database.h",
    "daemon/include/list_database.h",
    "daemon/include/transaction.h",
};

fn format_code(b: *std.Build, check: bool) void {
    const files = daemon_files ++ test_files ++ header_files;

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
