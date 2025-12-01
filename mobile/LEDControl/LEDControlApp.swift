import SwiftUI

@main
struct LEDControlApp: App {
    @StateObject private var bleManager = BLEManager()

    var body: some Scene {
        WindowGroup {
            ContentView(bleManager: bleManager)
        }
    }
}
