const std = @import("std");
const Dir = @import("fs/Dir.zig");

const TaskNode = struct { // Basically a Node
    source: ?*Task = null,
    priority: u8,
};

fn TaskQueue(path: []const u8, cwd: Dir) !TaskNode {
    const file = try cwd.openFile(path, .{ .read_only = true });
    defer file.close();

    var reader = std.io.bufferedReader(file.reader()).reader();
    var line_buffer: [1024]u8 = undefined;

    while(try reader.readUntilDelimiterOrEof(&line_buffer, '\n')) |line| {

    }
}

const Make = struct {

};

pub fn build(b: *std.build.Builder) void {

}
