# Enterprise networks

Network Settings can create user-restricted Wi-Fi and wired 802.1X profiles directly through NetworkManager D-Bus. Choose a nearby enterprise network, New enterprise Wi-Fi, or enable 802.1X while editing a wired profile. Saving creates a profile; select Connect under Saved connections to activate it.

Supported authentication:

- PEAP with MSCHAPv2 or GTC.
- TTLS with PAP, CHAP, MSCHAP, or MSCHAPv2 inside its TLS tunnel.
- TLS with a client certificate and private key, including an encrypted-key password.

Enter the identity, optional anonymous identity, CA certificate path, and authentication server domain supplied by the network administrator. A CA certificate and server domain are mandatory; there is no verification-off option. The domain is matched as a DNS suffix. Certificate/key files must remain at their selected absolute local paths. NetworkManager validates their format and the server certificate on activation. A client certificate and key may refer to the same PKCS#12 file. Harbor checks readability and limits files to 5 MB but does not parse cryptographic material.

Blank password fields preserve saved credentials while editing; missing credentials can be requested by the existing trusted NetworkManager SecretAgent on explicit activation. New password values are passed through D-Bus to NetworkManager, never command arguments or logs. New profiles are restricted to the current user. Other existing connection settings and secrets are preserved. Existing EAP methods outside the supported set, multiple EAP methods, and phase2-autheap configurations remain externally managed and are preserved when IP settings are edited. Existing blob or token certificates are retained when their path field is blank. Hardware token enrollment, certificate issuance, inner EAP-TLS configuration, and certificate browsing are not implemented.

IPv4/IPv6 addressing, gateway, and DNS remain available in the same native editor. Labels support English and Arabic and follow Harbor's light/dark theme.

Verification uses a private D-Bus NetworkManager fixture and temporary certificate paths; it does not change host networking. Coverage includes Wi-Fi/wired serialization, TLS/PEAP/TTLS, required trust fields, invalid paths and inner methods, preservation/replacement of stored secrets, and TLS key-password prompts. A real managed network remains necessary for end-to-end certificate authentication testing.

API reference: [NetworkManager 802.1X settings](https://networkmanager.pages.freedesktop.org/NetworkManager/NetworkManager/settings-802-1x.html). The D-Bus file scheme is UTF-8 `file://` plus an absolute path and terminating NUL byte.
