* most of the interesting code is in `do_port_action`
* remember that jack callbacks are in a separate thread; it's therefore needed
  the usage of thread-safe message-passing structures
* `pipe.{c,h}` is stolen from https://github.com/cgaebel/pipe as a thread-safe
  queue
* metro.c is there only for testing purposes, and is actually copied from jack
  examples


vim: set ft=markdown:
