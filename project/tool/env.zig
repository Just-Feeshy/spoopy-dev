const std = @import("std");
const target = @import("builtin").target;

pub const isWindows = target.os.tag == .windows;
