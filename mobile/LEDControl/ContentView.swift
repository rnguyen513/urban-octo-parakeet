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
                        VStack(spacing: 10) {
                            HStack(spacing: 15) {
                                Image(systemName: batteryIcon(for: bleManager.batteryPercentage))
                                    .font(.system(size: 30))
                                    .foregroundColor(batteryColor(for: bleManager.batteryPercentage))

                                VStack(alignment: .leading, spacing: 4) {
                                    Text("\(bleManager.batteryPercentage)%")
                                        .font(.title2)
                                        .fontWeight(.bold)
                                    Text("\(bleManager.batteryVoltage)V")
                                        .font(.subheadline)
                                        .foregroundColor(.secondary)
                                }
                            }
                        }
                        .padding()
                        .frame(maxWidth: .infinity)
                        .background(batteryColor(for: bleManager.batteryPercentage).opacity(0.1))
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

    // Helper function to get appropriate battery icon
    private func batteryIcon(for percentage: Int) -> String {
        switch percentage {
        case 75...100:
            return "battery.100"
        case 50..<75:
            return "battery.75"
        case 25..<50:
            return "battery.50"
        case 1..<25:
            return "battery.25"
        default:
            return "battery.0"
        }
    }

    // Helper function to get battery color
    private func batteryColor(for percentage: Int) -> Color {
        switch percentage {
        case 50...100:
            return .green
        case 20..<50:
            return .orange
        default:
            return .red
        }
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

            // Preview: Connected - Full battery (4.2V = 100%)
            ContentView(bleManager: MockBLEManager(simulatedState: .connected, simulatedVoltage: "4.20"))
                .previewDisplayName("Connected - Full")

            // Preview: Connected - Medium battery (3.7V = 58%)
            ContentView(bleManager: MockBLEManager(simulatedState: .connected, simulatedVoltage: "3.70"))
                .previewDisplayName("Connected - Medium")

            // Preview: Connected - Low battery (3.3V = 25%)
            ContentView(bleManager: MockBLEManager(simulatedState: .connected, simulatedVoltage: "3.30"))
                .previewDisplayName("Connected - Low")
        }
    }
}
