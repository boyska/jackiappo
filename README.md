JACKIAPPO
=========

* Watch for new ports and make new connections as soon as they are available
* According to a simple file

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

* Make thread safe queue to pass messages from callbacks to worker
* Reload config file on SIGHUP
* Make rules.c with rule-port matching function



vim: set et ts=4 sw=4:
