# Capteur géolocalisé connecté de la qualité de l’air

Projet réalisé dans le cadre du cours d'IoT à l'INSA Rennes par Pierre-Antoine CABARET, Nicolas FOUQUÉ, Clara GAOUYER et Emilie HUMMEL.

## Résumé
Ce projet consiste donc en la récupération de données à l'aide d'un M5Stack et de divers capteurs, qui sont enregistrées sur une carte micro-SD.
Les données peuvent ensuite être envoyées sur un serveur via Wi-Fi, ou bien récupérées manuellement en se connectant au M5Stack.
Une fois les données traitées, elles peuvent être visualisées sur une page web proposant diverses statistiques.

## Matériel utilisé
- un module GPS M003 pour M5Stack (avec une antenne externe fournie) ;
- un capteur de qualité de l'air PMSA003 ;
- un capteur de température et d'humidité Grove 101020074 ;
- une carte micro-SD.

## Fonctionnement

### M5stack
- Au démarrage ou lorsqu’on appuie sur le bouton 1, le M5Stack enregistre les données récupérées par les capteurs toutes les 10 secondes.

- Lorsqu’on appuie sur le bouton 2, il tente de se connecter en Wi-Fi et en cas de succès, envoie les dernières données recueillies à notre serveur web.

- Lorsqu’on appuie sur le bouton 3, le M5Stack passe en mode serveur et permet aux utilisateurs à proximité de s’y connecter en Wi-Fi et de consulter ou télécharger les données recueillies.

### API
Pour recueillir les données recueillies, nous avons créé un serveur web avec une API en Go. Cette API récupère les fichiers CSV générés, vérifie leur validité (supprime les valeurs aberrantes ou sans position GPS) et les insère dans une base de données MySQL.

### Interface
Grafana est un outil de visualisation de données. Il peut être relié à une base de données et permet d’organiser sous forme de dashboard des graphes permettant d’interpréter ces données.
Nous avons donc fait une page web contenant différents graphes sur la température, la pollution de l’air en fonction des différents types de particules, des cartes permettant de représenter la répartition des zones plus ou moins polluées,...
