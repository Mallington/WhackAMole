# WhackAMole
## "Created for CompSci Year 1 Semester 1 Coursework 1"

Here is a brief definition of the program as quoted from my report:
"Whack-A-Mole[1] is a simple arcade game where typically the user would have some form of hammer, and would have to whack moles as they appear from a series of holes. The moles would appear from the holes at random times in random locations, and the player would have to hit them all, their score increasing with each successful hit. In our implementation of this game, there are 3 LED lights to represent the moles, and a single button to whack the moles. There’s also a white LED to show when the user has successfully gained a point."

This repository contains the code used for the project, along with the video and documentation we had to do for the Coursework

Here is a very brief video of how it works:

[![Whack a mole](https://img.youtube.com/vi/9QGh1iv4xlI/0.jpg)](https://www.youtube.com/watch?v=9QGh1iv4xlI)

Here are the final schematics:
![](https://raw.githubusercontent.com/Mallington/WhackAMole/master/Screenshot%202018-11-22%20at%2020.30.46.png)

Some challenges we faced during development:
- Making stable coms between the two Arduinos. This was eventually fixed by using Boolean statements which waited for specific characters,
e.g. "$"
- Working on it together, as for the majority of the time we were very far away from each other
