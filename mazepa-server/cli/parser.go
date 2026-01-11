package cli

import (
	"mazepa-server/icmp"

	"github.com/spf13/cobra"
)

func GetParser(opts *MazepaOptions) *cobra.Command {

	var rootCmd = &cobra.Command{
		Use:   "run",
		Short: "Run the server",
		Long:  "Runs the ICMP listener & display all received keys on stdout",
		Args:  cobra.MinimumNArgs(0),
		Run: func(cmd *cobra.Command, args []string) {

			icmp.ListenAndPrint(opts.IP)
		},
	}

	defaults := GetDefaultOptions()
	rootCmd.Flags().StringVarP(&opts.IP, "ip", "i", defaults.IP, "Address to listen on (IPv4 or IPv6)")
	return rootCmd
}
