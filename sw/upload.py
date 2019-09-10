import uavcan
import argparse


# based on
# https://github.com/OpenMotorDrive/framework/blob/ebd2e9277bf4fbbcd37eba4b7b68ffffb75027d4/tools/uavcan_upload.py


parser = argparse.ArgumentParser()
parser.add_argument('--bus', default='can0', help="the can bus device (default: can0)")
parser.add_argument('--node-id', required=True)
parser.add_argument('--host-node-id', default=126)
parser.add_argument('firmware_file')
args = parser.parse_args()

update_complete = False
update_started = False

def monitor_update_handler(e):
    global update_complete, update_started
    if e.event_id == node_monitor.UpdateEvent.EVENT_ID_INFO_UPDATE:
        if e.entry.node_id == int(args.node_id):
            if e.entry.status.mode == e.entry.status.MODE_OPERATIONAL:
                print("found node in mode OPERATIONAL")
            elif e.entry.status.mode == e.entry.status.MODE_INITIALIZATION:
                print("found node in mode INITIALIZATION")
            elif e.entry.status.mode == e.entry.status.MODE_MAINTENANCE:
                print("found node in mode MAINTENANCE")
            elif e.entry.status.mode == e.entry.status.MODE_SOFTWARE_UPDATE:
                print("found node in mode SOFTWARE_UPDATE")
            else:
                print("found node in unknown mode")

            if e.entry.status.mode != e.entry.status.MODE_SOFTWARE_UPDATE or not update_started:
                if update_started == True:
                    print('update complete')
                    update_complete = True
                    return
                print('starting software update...')
                req_msg = uavcan.protocol.file.BeginFirmwareUpdate.Request(source_node_id=node.node_id, image_file_remote_path=uavcan.protocol.file.Path(path=args.firmware_file))
                node.request(req_msg, e.entry.node_id, update_response_handler)
                update_started = True



def update_response_handler(e):
    assert e.response.error == e.response.ERROR_OK

node = uavcan.make_node(args.bus)
node.node_id = args.host_node_id

print("firmware: %s" % args.firmware_file)
print("target node: %s" % args.node_id)

file_server = uavcan.app.file_server.FileServer(node, [args.firmware_file])
node_monitor = uavcan.app.node_monitor.NodeMonitor(node)
allocator = uavcan.app.dynamic_node_id.CentralizedServer(node, node_monitor)
node_monitor.add_update_handler(monitor_update_handler)

# discover nodes
import time
tstart = time.time()
while time.time()-tstart < float(5):
    try:
        node.spin(1)
    except Exception:
        pass

# wait for updates to complete
while not update_complete:
    try:
        node.spin(1)
    except Exception:
        pass
