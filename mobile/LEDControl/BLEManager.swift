import Foundation
import CoreBluetooth

// Protocol for BLE Manager to enable mocking for previews
protocol BLEManagerProtocol: ObservableObject {
    var isScanning: Bool { get set }
    var isConnected: Bool { get set }
    var discoveredPeripherals: [CBPeripheral] { get set }
    var statusMessage: String { get set }
    var ledState: Bool { get set }

    func startScanning()
    func stopScanning()
    func connect(to peripheral: CBPeripheral)
    func disconnect()
    func setLEDState(_ state: Bool)
}

class BLEManager: NSObject, BLEManagerProtocol, ObservableObject {
    // BLE UUIDs matching ESP32
    private let serviceUUID = CBUUID(string: "4fafc201-1fb5-459e-8fcc-c5c9c331914b")
    private let characteristicUUID = CBUUID(string: "beb5483e-36e1-4688-b7f5-ea07361b26a8")

    // Published properties for UI updates
    @Published var isScanning = false
    @Published var isConnected = false
    @Published var discoveredPeripherals: [CBPeripheral] = []
    @Published var statusMessage = "Ready to scan"
    @Published var ledState = false

    // Core Bluetooth objects
    private var centralManager: CBCentralManager!
    private var connectedPeripheral: CBPeripheral?
    private var ledCharacteristic: CBCharacteristic?

    override init() {
        super.init()
        centralManager = CBCentralManager(delegate: self, queue: nil)
    }

    // Start scanning for ESP32 device
    func startScanning() {
        guard centralManager.state == .poweredOn else {
            statusMessage = "Bluetooth is not available"
            return
        }

        discoveredPeripherals.removeAll()
        isScanning = true
        statusMessage = "Scanning for ESP32-LED..."
        centralManager.scanForPeripherals(withServices: [serviceUUID], options: nil)

        // Stop scanning after 10 seconds
        DispatchQueue.main.asyncAfter(deadline: .now() + 10) { [weak self] in
            self?.stopScanning()
        }
    }

    func stopScanning() {
        isScanning = false
        centralManager.stopScan()
        if !isConnected {
            statusMessage = discoveredPeripherals.isEmpty ? "No devices found" : "Scan complete"
        }
    }

    // Connect to a peripheral
    func connect(to peripheral: CBPeripheral) {
        stopScanning()
        connectedPeripheral = peripheral
        peripheral.delegate = self
        statusMessage = "Connecting to \(peripheral.name ?? "device")..."
        centralManager.connect(peripheral, options: nil)
    }

    // Disconnect from peripheral
    func disconnect() {
        guard let peripheral = connectedPeripheral else { return }
        centralManager.cancelPeripheralConnection(peripheral)
    }

    // Send LED state to ESP32
    func setLEDState(_ state: Bool) {
        guard let characteristic = ledCharacteristic,
              let peripheral = connectedPeripheral else {
            statusMessage = "Not connected to device"
            return
        }

        let value: UInt8 = state ? 1 : 0
        let data = Data([value])
        peripheral.writeValue(data, for: characteristic, type: .withResponse)
        ledState = state
        statusMessage = "LED turned \(state ? "ON" : "OFF")"
    }
}

// MARK: - CBCentralManagerDelegate
extension BLEManager: CBCentralManagerDelegate {
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        switch central.state {
        case .poweredOn:
            statusMessage = "Bluetooth is ready"
        case .poweredOff:
            statusMessage = "Bluetooth is powered off"
        case .unauthorized:
            statusMessage = "Bluetooth permission denied"
        case .unsupported:
            statusMessage = "Bluetooth not supported"
        default:
            statusMessage = "Bluetooth unavailable"
        }
    }

    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        // Add discovered peripheral if not already in list
        if !discoveredPeripherals.contains(where: { $0.identifier == peripheral.identifier }) {
            discoveredPeripherals.append(peripheral)
            statusMessage = "Found \(peripheral.name ?? "Unknown")"
        }
    }

    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        isConnected = true
        statusMessage = "Connected to \(peripheral.name ?? "device")"
        peripheral.discoverServices([serviceUUID])
    }

    func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        isConnected = false
        ledCharacteristic = nil
        connectedPeripheral = nil
        ledState = false
        statusMessage = "Disconnected"
    }

    func centralManager(_ central: CBCentralManager, didFailToConnect peripheral: CBPeripheral, error: Error?) {
        statusMessage = "Failed to connect: \(error?.localizedDescription ?? "Unknown error")"
        connectedPeripheral = nil
    }
}

// MARK: - CBPeripheralDelegate
extension BLEManager: CBPeripheralDelegate {
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        guard let services = peripheral.services else { return }

        for service in services {
            if service.uuid == serviceUUID {
                peripheral.discoverCharacteristics([characteristicUUID], for: service)
            }
        }
    }

    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        guard let characteristics = service.characteristics else { return }

        for characteristic in characteristics {
            if characteristic.uuid == characteristicUUID {
                ledCharacteristic = characteristic
                statusMessage = "Ready to control LED"
            }
        }
    }

    func peripheral(_ peripheral: CBPeripheral, didWriteValueFor characteristic: CBCharacteristic, error: Error?) {
        if let error = error {
            statusMessage = "Write error: \(error.localizedDescription)"
        }
    }
}

// MARK: - Mock BLE Manager for Previews
class MockBLEManager: BLEManagerProtocol, ObservableObject {
    @Published var isScanning = false
    @Published var isConnected = false
    @Published var discoveredPeripherals: [CBPeripheral] = []
    @Published var statusMessage = "Preview Mode - Ready to scan"
    @Published var ledState = false

    init(simulatedState: SimulatedState = .disconnected) {
        // Set up initial state based on simulation mode
        switch simulatedState {
        case .disconnected:
            isConnected = false
            statusMessage = "Preview Mode - Ready to scan"
        case .scanning:
            isScanning = true
            statusMessage = "Preview Mode - Scanning..."
        case .connected:
            isConnected = true
            statusMessage = "Preview Mode - Connected to SKIBIDI-led"
        }
    }

    enum SimulatedState {
        case disconnected
        case scanning
        case connected
    }

    func startScanning() {
        isScanning = true
        statusMessage = "Preview Mode - Scanning for devices..."

        // Simulate finding a device after a delay
        DispatchQueue.main.asyncAfter(deadline: .now() + 2) { [weak self] in
            self?.isScanning = false
            self?.statusMessage = "Preview Mode - simulated scanning stopped. Device found"
        }
    }

    func stopScanning() {
        isScanning = false
        statusMessage = "Preview Mode - Scan stopped"
    }

    func connect(to peripheral: CBPeripheral) {
        statusMessage = "Preview Mode - Connecting..."

        // Simulate connection after delay
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) { [weak self] in
            self?.isConnected = true
            self?.statusMessage = "Preview Mode - Connected to skibidi-led"
        }
    }

    func disconnect() {
        isConnected = false
        ledState = false
        statusMessage = "Preview Mode - Disconnected"
    }

    func setLEDState(_ state: Bool) {
        ledState = state
        statusMessage = "Preview Mode - LED turned \(state ? "ON" : "OFF")"
    }
}
