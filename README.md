# TP_AAA_Bonnet_Terlinden

## Objectif

A partir d'un hacheur complet et d'une carte Nucleo-STM32G474RE, nous devons:

- Réaliser la commande des 4 transistors du hacheur en commande complémentaire décalée,
- Faire l'acquisition des différents capteurs,
- Réaliser l'asservissement en temps réel.

## Pinout
Voici les deux schémas de brochage utilisés pour le projet :

- ![Pinout 1](ressource/pinout1.jpg)  
	*Figure 1 — Pinout droit(fichier `ressource/pinout1.jpg`).*

- ![Pinout 2](ressource/pinout2.jpg)  
	*Figure 2 — Pinout gauche(fichier `ressource/pinout2.jpg`).*

- ![Chip Pinout](ressource/chippinout.jpg)  
	*Figure 3 — Pinout de la chip(fichier `ressource/chippinout.jpg`).*

## Commande MCC basique

Objectifs :

- Générer 4 PWM en complémentaire décalée pour contrôler en boucle ouverte le moteur en respectant le cahier des charges,
- Inclure le temps mort,
- Vérifier les signaux de commande à l'oscilloscope,
- Prendre en main le hacheur,
- Faire un premier essai de commande moteur.

On crée deffirentes pwm d'une fréquence de 20kHz:

- ![Visualisation d'une pwm](ressource/sonde1signal.png)  
	*Figure 4 — Visualisation d'une pwm sur TIM_CHANNEL_1 à 60%(fichier `ressource/sonde1signal.png`).*

- ![Visualisation de 2 pwm](ressource/sonde2signal.png)  
	*Figure 4 — Visualisation d'une pwm sur TIM_CHANNEL_1 et de sa complémentaire TIM_CHANNEL_1N à 60%(fichier `ressource/sonde2signal.png`).*

- ![Visualisation de 4 pwm](ressource/sonde4signal.png)  
	*Figure 4 — Visualisation de 4 pwm à 60%(fichier `ressource/sonde4signal.png`).*