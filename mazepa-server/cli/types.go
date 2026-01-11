package cli

type MazepaOptions struct {
	IP string
}

func GetDefaultOptions() MazepaOptions {
	return MazepaOptions{
		IP: "127.0.0.1",
	}
}
