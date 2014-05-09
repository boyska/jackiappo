JACKIAPPO
=========

jackiappo watches jack events and reacts to them. By now, it watches for new
ports and connects to them.

Use cases
-----------

Some jack clients are hardly configurable, which means that one must manually
connect them to desired output ports. Using jackiappo, this is all done
automatically.


Build
------

Just run `make` and you're done.
It depends on jack (obviously) and
[libconfig](http://www.hyperrealm.com/libconfig/)


Run
----

```sh
./jackiappo -c config.cfg
```

File format
-----------

According to libconfig [1]

newports is a list whose element are groups called "newportRules"
a portRule needs to have a "from" and a "to" setting inside.
A portRule can have these settings: name (other can happen). All settings are
in conjunction, so more setting restrict specification.
When a new port appears, every "from" rule is tested. If the from rule matches, it is connected to every port matching the "to" rule.

Example:

    newports = (
        {
            from = { name="PortAudio:in_1" }
            to = { name="Non-Mixer (rec):out_1" }
        },
        {
            from = { name="PortAudio:in_2" }
            to = { name="Non-Mixer (rec):out_2" }
        }
    )

[1] http://www.hyperrealm.com/libconfig

TODO
-----

* Reload config file on SIGHUP
* Use globals ONLY from main thread



vim: set et ts=4 sw=4:
