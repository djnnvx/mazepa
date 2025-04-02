package cli

type MazepaOptions struct {

	/// Address to listen on (only IPv4 supported for now)
	IP string
}

func GetDefaultOptions() MazepaOptions {

	opts := MazepaOptions{
		IP: "127.0.0.1",
	}

	return opts
}
