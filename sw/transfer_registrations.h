//REGISTER_TRANSFER(CanardTransferTypeBroadcast, HOMEAUTOMATION_TIME_DATA_TYPE_ID, HOMEAUTOMATION_TIME_DATA_TYPE_SIGNATURE, on_time_data)
//REGISTER_TRANSFER(CanardTransferTypeBroadcast, HOMEAUTOMATION_ENVIRONMENT_DATA_TYPE_ID, HOMEAUTOMATION_ENVIRONMENT_DATA_TYPE_SIGNATURE, on_environment_data)

// enable these for HALLWAY_LIGHT
//REGISTER_TRANSFER(CanardTransferTypeBroadcast, HOMEAUTOMATION_TIME_DATA_TYPE_ID, HOMEAUTOMATION_TIME_DATA_TYPE_SIGNATURE, on_time_data)
//REGISTER_TRANSFER(CanardTransferTypeBroadcast, HOMEAUTOMATION_BATHROOMSTATUS_ID, HOMEAUTOMATION_BATHROOMSTATUS_SIGNATURE, on_bathroom_data)

// enable these for K22_LIGHT
REGISTER_TRANSFER(CanardTransferTypeBroadcast, HOMEAUTOMATION_BATHROOMSTATUS_ID, HOMEAUTOMATION_BATHROOMSTATUS_SIGNATURE, on_bathroom_data)

// enable these for BOILER
//REGISTER_TRANSFER(CanardTransferTypeBroadcast, HOMEAUTOMATION_OBIS_DATA_TYPE_ID, HOMEAUTOMATION_OBIS_DATA_TYPE_SIGNATURE, on_obis_data)
//REGISTER_TRANSFER(CanardTransferTypeBroadcast, HOMEAUTOMATION_BOILERSTATUS_ID, HOMEAUTOMATION_BOILERSTATUS_SIGNATURE, on_boilerstatus_data)

// enable these for VIESSMANN_TRIMATIK
// REGISTER_TRANSFER(CanardTransferTypeBroadcast, HOMEAUTOMATION_TIME_DATA_TYPE_ID, HOMEAUTOMATION_TIME_DATA_TYPE_SIGNATURE, on_time_data)