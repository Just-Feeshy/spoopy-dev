const std = @import("std");
const Env = @import("env.zig");

pub const char = if (Env.isWindows) u16 else u8;
pub const string = []const char;
pub const stringZ = [:0]const char;
pub const mut_string = []char;
