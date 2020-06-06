#!/usr/bin/python3
import sys
import subprocess
import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk


valveTime = 4000


class ShotBotApp(Gtk.Application):
    def __init__(self):
        super(ShotBotApp, self).__init__(application_id='com.github.rookies.shotbot')

    def on_activate(self, app):
        self.window = Gtk.ApplicationWindow.new(self)
        self.window.set_title('ShotBot')

        vbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL,
                       homogeneous=True, spacing=0)
        self.window.add(vbox)

        self.buttons = []
        for i in range(3):
            hbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL,
                           homogeneous=True, spacing=0)
            vbox.add(hbox)

            for j in range(2):
                count = (j*3) + i + 1
                button = Gtk.Button(label=str(count))
                button.connect('pressed', self.on_buttonclick, count)
                hbox.add(button)
                self.buttons.append(button)

        self.window.show_all()

    def on_buttonclick(self, button, count):
        dialog = Gtk.MessageDialog(
            parent=self.window,
            modal=True,
            message_type=Gtk.MessageType.QUESTION,
            buttons=Gtk.ButtonsType.OK_CANCEL,
            text=f'Wirklich {count} Pfeffi ausschenken?'
        )
        response = dialog.run()
        dialog.destroy()

        if response == Gtk.ResponseType.OK:
            for button in self.buttons:
                button.set_sensitive(False)

            self.serve(count)

            for button in self.buttons:
                button.set_sensitive(True)

    def serve(self, count):
        subprocess.run(['./main.py', '/dev/ttyUSB0', str(valveTime), str(count)])

    def run(self, argv):
        self.connect('activate', self.on_activate)
        return super(ShotBotApp, self).run(argv)

if __name__ == '__main__':
    app = ShotBotApp()
    SystemExit(app.run(sys.argv))
