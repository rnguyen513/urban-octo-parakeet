import SwiftUI

struct ContentView<Manager: BLEManagerProtocol>: View {
    @ObservedObject var bleManager: Manager

    var body: some View {
        NavigationView {
            VStack(spacing: 30) {
                // Status indicator
                HStack {
                    Circle()
                        .fill(bleManager.isConnected ? Color.green : Color.red)
                        .frame(width: 12, height: 12)
                    Text(bleManager.statusMessage)
                        .font(.subheadline)
                        .foregroundColor(.secondary)
                }
                .padding()
                .frame(maxWidth: .infinity)
                .background(Color.gray.opacity(0.1))
                .cornerRadius(10)

                // Device list
                if !bleManager.isConnected {
                    VStack(alignment: .leading, spacing: 15) {
                        Text("Available Devices")
                            .font(.headline)

                        if bleManager.discoveredPeripherals.isEmpty {
                            Text("No devices found")
                                .font(.subheadline)
                                .foregroundColor(.secondary)
                                .padding()
                        } else {
                            ForEach(bleManager.discoveredPeripherals, id: \.identifier) { peripheral in
                                Button(action: {
                                    bleManager.connect(to: peripheral)
                                }) {
                                    HStack {
                                        VStack(alignment: .leading) {
                                            Text(peripheral.name ?? "Unknown Device")
                                                .font(.headline)
                                            Text(peripheral.identifier.uuidString)
                                                .font(.caption)
                                                .foregroundColor(.secondary)
                                        }
                                        Spacer()
                                        Image(systemName: "chevron.right")
                                            .foregroundColor(.blue)
                                    }
                                    .padding()
                                    .background(Color.blue.opacity(0.1))
                                    .cornerRadius(10)
                                }
                                .buttonStyle(PlainButtonStyle())
                            }
                        }
                    }
                    .padding()

                    // Scan button
                    Button(action: {
                        if bleManager.isScanning {
                            bleManager.stopScanning()
                        } else {
                            bleManager.startScanning()
                        }
                    }) {
                        HStack {
                            Image(systemName: bleManager.isScanning ? "stop.circle" : "antenna.radiowaves.left.and.right")
                            Text(bleManager.isScanning ? "Stop Scanning" : "Scan for Devices")
                        }
                        .font(.headline)
                        .foregroundColor(.white)
                        .padding()
                        .frame(maxWidth: .infinity)
                        .background(bleManager.isScanning ? Color.red : Color.blue)
                        .cornerRadius(10)
                    }
                    .padding()
                }

                // LED control (only shown when connected)
                if bleManager.isConnected {
                    VStack(spacing: 20) {
                        // Battery indicator
                        HStack {
                            Image(systemName: "battery.100")
                                .foregroundColor(.green)
                            Text("\(bleManager.batteryVoltage)V")
                                .font(.headline)
                                .fontWeight(.semibold)
                        }
                        .padding()
                        .frame(maxWidth: .infinity)
                        .background(Color.green.opacity(0.1))
                        .cornerRadius(10)

                        Image(systemName: bleManager.ledState ? "lightbulb.fill" : "lightbulb")
                            .resizable()
                            .scaledToFit()
                            .frame(width: 100, height: 100)
                            .foregroundColor(bleManager.ledState ? .yellow : .gray)
                            .padding()

                        Text("LED Control")
                            .font(.title2)
                            .fontWeight(.bold)

                        Toggle(isOn: Binding(
                            get: { bleManager.ledState },
                            set: { newValue in
                                bleManager.setLEDState(newValue)
                            }
                        )) {
                            Text(bleManager.ledState ? "ON" : "OFF")
                                .font(.headline)
                        }
                        .toggleStyle(SwitchToggleStyle(tint: .blue))
                        .padding()
                        .background(Color.gray.opacity(0.1))
                        .cornerRadius(10)

                        Button(action: {
                            bleManager.disconnect()
                        }) {
                            HStack {
                                Image(systemName: "xmark.circle")
                                Text("Disconnect")
                            }
                            .font(.headline)
                            .foregroundColor(.white)
                            .padding()
                            .frame(maxWidth: .infinity)
                            .background(Color.red)
                            .cornerRadius(10)
                        }
                        .padding(.top, 20)
                    }
                    .padding()
                }

                Spacer()
            }
            .background(.red)
            .padding()
        }
        .ignoresSafeArea()
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        Group {
            // Preview: Disconnected state
            ContentView(bleManager: MockBLEManager(simulatedState: .disconnected))
                .previewDisplayName("Disconnected")

            // Preview: Scanning state
            ContentView(bleManager: MockBLEManager(simulatedState: .scanning))
                .previewDisplayName("Scanning")

            // Preview: Connected state
            ContentView(bleManager: MockBLEManager(simulatedState: .connected))
                .previewDisplayName("Connected")
        }
    }
}
