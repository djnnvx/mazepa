
/*
    TODO(djnn):

    - read from memory & retrieve config

*/

pub struct Config {

    pub daemon_enabled: bool,
    pub callback_ip: String,
}


impl Config {
    pub fn new() -> Self {

        #[cfg(debug_assertions)]
        Config{
            callback_ip: "10.0.0.2".to_string(),
            daemon_enabled: false,
        };

        Config{
            callback_ip: "10.0.0.2".to_string(),
            daemon_enabled: false,
        }
    }
}

