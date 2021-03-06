import dbus
import dbus.service
import testutils
import sys
import time
from bleAdapter import bleAdapter

ROOT_AGENT_PATH = "/org/bluez"
TEST_AGENT_PATH = "/org/bluez/my_bluetooth_agent"
CAPABILITIES = "DisplayYesNo"

def ask(prompt):
	try:
		test  = raw_input(prompt)
		print("just input" + test)
		return test 
	except:
		return input(prompt)

class Rejected(dbus.DBusException):
	_dbus_error_name = "org.bluez.Error.Rejected"

class Agent(dbus.service.Object):
	def __init__(self, object_path):
		dbus.service.Object.__init__(self, dbus.SystemBus(), object_path)


	@dbus.service.method(dbus_interface=testutils.AGENT_INTERFACE, in_signature="", out_signature="")
	def Release(self):
		print("Release")
		

	@dbus.service.method(dbus_interface=testutils.AGENT_INTERFACE, in_signature="os", out_signature="")
	def AuthorizeService(self, device, uuid):
		print("AuthorizeService (%s, %s)" % (device, uuid))
		sys.stdout.flush()
		authorize = ask("Authorize connection (yes/no): ")
		if (authorize == "yes"):
			return
		raise Rejected("Connection rejected by user")

	@dbus.service.method(dbus_interface=testutils.AGENT_INTERFACE, in_signature="o", out_signature="s")
	def RequestPinCode(self, device):
		print("RequestPinCode (%s)" % (device))
		sys.stdout.flush()
		set_trusted(device)
		return ask("Enter PIN Code: ")

	@dbus.service.method(dbus_interface=testutils.AGENT_INTERFACE, in_signature="o", out_signature="u")
	def RequestPasskey(self, device):
		print("RequestPasskey (%s)" % (device))
		sys.stdout.flush()
		set_trusted(device)
		passkey = ask("Enter passkey: ")
		return dbus.UInt32(passkey)

	@dbus.service.method(dbus_interface=testutils.AGENT_INTERFACE, in_signature="ouq", out_signature="")
	def DisplayPasskey(self, device, passkey, entered):
		print("DisplayPasskey (%s, %06u entered %u)" % (device, passkey, entered))
		sys.stdout.flush()

	@dbus.service.method(dbus_interface=testutils.AGENT_INTERFACE, in_signature="os", out_signature="")
	def DisplayPinCode(self, device, pincode):
		print("DisplayPinCode (%s, %s)" % (device, pincode))
		sys.stdout.flush()

	@dbus.service.method(dbus_interface=testutils.AGENT_INTERFACE, in_signature="ou", out_signature="")
	def RequestConfirmation(self, device, passkey):
		return

	@dbus.service.method(dbus_interface=testutils.AGENT_INTERFACE, in_signature="o", out_signature="")
	def RequestAuthorization(self, device):
		print("RequestAuthorization (%s)" % (device))
		sys.stdout.flush()
		auth = ask("Authorize? (yes/no): ")
		if (auth == "yes"):
			return
		raise Rejected("Pairing rejected")

	@dbus.service.method(dbus_interface=testutils.AGENT_INTERFACE, in_signature="", out_signature="")
	def Cancel(self):
		print("Cancel")
		sys.stdout.flush()


def createSecurityAgent(paramCapabilities = "DisplayYesNo", agent = None):
	CAPABILITIES = paramCapabilities

	if(agent == None):
		agent = Agent(TEST_AGENT_PATH)

	while True:
		try:
			bus = dbus.SystemBus()
			obj = bus.get_object(testutils.SERVICE_NAME, ROOT_AGENT_PATH);
			manager = dbus.Interface(obj, testutils.AGENT_MANAGER_INTERFACE)
			manager.RegisterAgent(TEST_AGENT_PATH, CAPABILITIES)
			manager.RequestDefaultAgent(TEST_AGENT_PATH)
			sys.stdout.flush()
			break
		except dbus.exceptions.DBusException as error:
			print("Agent could not get started:" + str(error))
			print("Retrying after 1s")
			sys.stdout.flush()
			time.sleep(1)
			pass

	return agent

def removeSecurityAgent():
	try:
		bus = dbus.SystemBus()
		obj = bus.get_object(testutils.SERVICE_NAME, ROOT_AGENT_PATH);
		manager = dbus.Interface(obj, testutils.AGENT_MANAGER_INTERFACE)
		manager.UnregisterAgent(TEST_AGENT_PATH)
	except dbus.exceptions.DBusException as error:
		print("Agent could not get started:" + str(error))
		sys.stdout.flush()
