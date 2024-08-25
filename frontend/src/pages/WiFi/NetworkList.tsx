import { type JSX } from "preact";
import { useEffect, useId, useState } from "preact/hooks";

import { APP_CONFIG } from "config";

interface NetworkListProps {
  selectedNetwork: string;
  setSelectedNetwork: (network: string) => void;
  isScanning: boolean;
  setIsScanning: (scanning: boolean) => void;
}

class WiFiNetworkData {
  ssid: string;
  rssi: number;
}

export function NetworkList({
  selectedNetwork,
  setSelectedNetwork,
  isScanning,
  setIsScanning,
}: NetworkListProps): JSX.Element {
  const id = useId();

  const [wifiNetworks, setWiFiNetworks] = useState<WiFiNetworkData[]>([]);

  async function updateNetworks(): Promise<void> {
    // Initiate a WiFi scan. This should return status code 202.
    const response = await fetch(APP_CONFIG.wifi_scan_path, { method: "POST" });
    if (response.status !== 202) {
      throw new Error(`HTTP Error ${response.status} ${response.statusText}`);
    }
    setIsScanning(true);
    setTimeout(getScannedWiFiNetworks, 1000);
  }

  async function getScannedWiFiNetworks(): Promise<void> {
    const response = await fetch(APP_CONFIG.wifi_scan_results_path);
    if (response.status === 202) {
      // still scanning
      setTimeout(getScannedWiFiNetworks, 1000);
      return;
    }
    if (!response.ok) {
      throw new Error(`HTTP Error ${response.status} ${response.statusText}`);
    }
    const data = await response.json();
    // Data is an array of objects with the following properties:
    // - ssid: string
    // - rssi: number
    // - encryption: number
    // - channel: number
    // - bssid: string

    // Store the unique ssids with their strongest rssi values as an array of
    // WiFiNetworkData objects, sorted by rssi.

    const uniqueNetworks = new Map<string, WiFiNetworkData>();
    for (const network of data.networks) {
      const ssid = network.ssid;
      const rssi = network.rssi;
      if (uniqueNetworks.has(ssid)) {
        const existingNetwork = uniqueNetworks.get(ssid);
        if (existingNetwork && existingNetwork.rssi < rssi) {
          existingNetwork.rssi = rssi;
        }
      } else {
        uniqueNetworks.set(ssid, { ssid, rssi });
      }
    }

    const sortedNetworks = Array.from(uniqueNetworks.values()).sort(
      (a, b) => b.rssi - a.rssi,
    );

    setWiFiNetworks(sortedNetworks);
    setIsScanning(false);
  }

  useEffect(() => {
    function conditionalUpdateNetworks(): void {
      if (!isScanning) {
        void updateNetworks();
      }
    }
    const interval = setInterval(conditionalUpdateNetworks, 15000); // Repeat every 15 seconds
    conditionalUpdateNetworks(); // Run once immediately

    return () => {
      clearInterval(interval); // Clear the interval when the component is unmounted
    };
  }, []);

  return (
    <ul
      className="list-group"
      id={`${id}-networks`}
      style={{ maxHeight: "400px" }}
    >
      {wifiNetworks.map((network) => (
        <NetworkListItem
          key={network.ssid}
          name={network.ssid}
          strength={network.rssi}
          active={network.ssid === selectedNetwork}
          setActive={() => {
            setSelectedNetwork(network.ssid);
          }}
        />
      ))}
    </ul>
  );

  interface NetworkListItemProps {
    name: string;
    strength: number;
    active: boolean;
    setActive: () => void;
  }

  function NetworkListItem({
    name,
    strength,
    active,
    setActive,
  }: NetworkListItemProps): JSX.Element {
    function handleSelect(event): void {
      event.target.active = true;
      setActive();
    }

    return (
      <li
        className={`list-group-item d-flex justify-content-between align-items-start${
          active ? " active" : ""
        }`}
        id={name}
        onClick={handleSelect}
      >
        <div className="ms-2 me-auto">{name}</div>
        <div style={{ fontSize: "smaller" }}> {strength}</div>
      </li>
    );
  }
}
