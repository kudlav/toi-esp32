# SOURCE OF CODE:
# https://scribles.net/creating-ble-gatt-server-uart-service-on-raspberry-pi/
import sys
import dbus, dbus.mainloop.glib
from gi.repository import GLib
from example_advertisement import Advertisement
from example_advertisement import register_ad_cb, register_ad_error_cb
from example_gatt_server import Service, Characteristic
from example_gatt_server import register_app_cb, register_app_error_cb

BLUEZ_SERVICE_NAME =           'org.bluez'
DBUS_OM_IFACE =                'org.freedesktop.DBus.ObjectManager'
LE_ADVERTISING_MANAGER_IFACE = 'org.bluez.LEAdvertisingManager1'
GATT_MANAGER_IFACE =           'org.bluez.GattManager1'
GATT_CHRC_IFACE =              'org.bluez.GattCharacteristic1'
SERVICE_UUID =            '180f'
COLOR_UUID =  '1110'
RED_UUID =  '1000'
GREEN_UUID =  '0100'
BLUE_UUID =  '0010'
LOCAL_NAME =                   'rpi-gatt-server'
mainloop = None


class NotCharacter(Characteristic):
    def __init__(self, bus, index, service):
        Characteristic.__init__(self, bus, index, COLOR_UUID,
                                ['notify'], service)
        self.notifying = False

    def send_tx(self):
        print('sendNotification')
        self.notifying = True
        if not self.notifying:
            return
        value = []
        for c in 'newSetup':
            value.append(dbus.Byte(c.encode()))
        self.PropertiesChanged(GATT_CHRC_IFACE, {'Value': value}, [])

    def StartNotify(self):
        if self.notifying:
            return
        self.notifying = True

    def StopNotify(self):
        if not self.notifying:
            return
        self.notifying = False




class ColorCharacter(Characteristic):
    
    def __init__(self, bus, index, service, charUUID, notF):
        Characteristic.__init__(self, bus, index, charUUID, ['write', 'read'], service)
        self.notify = notF
        self.intensity = 125

    def send_tx(self):
        print("SEND message")
        if not self.notifying:
            return
        value = []
        for c in str(intensity):
            value.append(dbus.Byte(c.encode()))
        self.PropertiesChanged(GATT_CHRC_IFACE, {'Value': value}, [])

    def ReadValue(self, options):
        return [self.intensity]


    def WriteValue(self, value, options):
        print('remote led: {}'.format(bytearray(value).decode()))
        self.intensity = 10
        self.notify.send_tx()



class RGBService(Service):
    def __init__(self, bus, index):
        Service.__init__(self, bus, index, SERVICE_UUID, True)

        notFunc = NotCharacter(bus, 0, self)
      
        self.add_characteristic(notFunc)
        self.add_characteristic(ColorCharacter(bus, 1, self, RED_UUID, notFunc))
        self.add_characteristic(ColorCharacter(bus, 2, self, GREEN_UUID, notFunc))
        self.add_characteristic(ColorCharacter(bus, 3, self, BLUE_UUID, notFunc))




class Application(dbus.service.Object):
    def __init__(self, bus):
        self.path = '/'
        self.services = []
        dbus.service.Object.__init__(self, bus, self.path)

    def get_path(self):
        return dbus.ObjectPath(self.path)

    def add_service(self, service):
        self.services.append(service)

    @dbus.service.method(DBUS_OM_IFACE, out_signature='a{oa{sa{sv}}}')
    def GetManagedObjects(self):
        response = {}
        for service in self.services:
            response[service.get_path()] = service.get_properties()
            chrcs = service.get_characteristics()
            for chrc in chrcs:
                response[chrc.get_path()] = chrc.get_properties()
        return response

class UartApplication(Application):
    def __init__(self, bus):
        Application.__init__(self, bus)
        self.add_service(RGBService(bus, 1))

class UartAdvertisement(Advertisement):
    def __init__(self, bus, index):
        Advertisement.__init__(self, bus, index, 'peripheral')
        self.add_service_uuid(SERVICE_UUID)
        self.add_local_name(LOCAL_NAME)
        self.include_tx_power = True

def find_adapter(bus):
    remote_om = dbus.Interface(bus.get_object(BLUEZ_SERVICE_NAME, '/'),
                               DBUS_OM_IFACE)
    objects = remote_om.GetManagedObjects()
    for o, props in objects.items():
        if LE_ADVERTISING_MANAGER_IFACE in props and GATT_MANAGER_IFACE in props:
            return o
        print('Skip adapter:', o)
    return None

def main():
    global mainloop
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = dbus.SystemBus()
    adapter = find_adapter(bus)
    if not adapter:
        print('BLE adapter not found')
        return

    service_manager = dbus.Interface(
                                bus.get_object(BLUEZ_SERVICE_NAME, adapter),
                                GATT_MANAGER_IFACE)
    ad_manager = dbus.Interface(bus.get_object(BLUEZ_SERVICE_NAME, adapter),
                                LE_ADVERTISING_MANAGER_IFACE)

    app = UartApplication(bus)
    adv = UartAdvertisement(bus, 0)

    mainloop = GLib.MainLoop()

    service_manager.RegisterApplication(app.get_path(), {},
                                        reply_handler=register_app_cb,
                                        error_handler=register_app_error_cb)
    ad_manager.RegisterAdvertisement(adv.get_path(), {},
                                     reply_handler=register_ad_cb,
                                     error_handler=register_ad_error_cb)
    try:
        mainloop.run()
    except KeyboardInterrupt:
        adv.Release()

if __name__ == '__main__':
    main()
