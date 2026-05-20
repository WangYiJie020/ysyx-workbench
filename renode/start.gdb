set architecture riscv:rv32

python
class Reconnect(gdb.Command):
    def __init__(self):
        super(Reconnect, self).__init__("reconnect", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        try:
            gdb.execute("disconnect", to_string=True)
        except gdb.error as e:
            if "target is `exec'" not in str(e):
                raise
        gdb.execute("target remote localhost:3333", from_tty=from_tty)

Reconnect()
end

target remote :3333
