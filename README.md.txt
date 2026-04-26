# 🏠 Domus Smart Control - ESP32

## 📌 Description
Système domotique intelligent basé sur ESP32 permettant de contrôler un éclairage RGB, une porte automatique via RFID, et une alarme de sécurité à travers une interface web embarquée.

## ⚙️ Fonctionnalités
- 🔐 Contrôle d'accès avec carte RFID
- 🚪 Ouverture automatique de porte (servo)
- 💡 Gestion LED RGB (manuel + automatique)
- 🌐 Interface web via WiFi (ESP32 en mode Access Point)
- 🚨 Détection de mouvement avec alarme (PIR + buzzer)

## 🧰 Composants utilisés
- ESP32
- Module RFID RC522
- Servo moteur
- LED RGB
- Capteur PIR
- Buzzer

## 🌐 Accès
- WiFi : ESP32-RGB
- Mot de passe : 12345678
- Adresse IP : 192.168.4.1

## 🔌 Fonctionnement
L’ESP32 agit comme un serveur web. L’utilisateur se connecte via WiFi et contrôle le système depuis un navigateur.  
Le RFID permet l’accès sécurisé, le servo ouvre la porte, et le capteur PIR active une alarme en cas de mouvement.

## 📁 Structure du projet
- /code → code Arduino (ESP32)
- /images → photos du montage

## 📷 Aperçu
(Ajoute ici une image de ton montage)

## 👨‍💻 Auteur
Étudiant en IoT - UAC