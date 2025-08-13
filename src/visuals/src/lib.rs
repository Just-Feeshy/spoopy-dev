use std::os::raw::{c_int};

#[unsafe(no_mangle)]
pub extern "C" fn simple_init() -> c_int {
    0
}
