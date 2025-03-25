**RxTrainer**

RxTrainer is a project developed as a semester-long project for an embedded systems course at Penn State.

The primary goal of this project is to use real-time constraints in an operating environment.

Broadly, this project simulates a nuclear reactor and coolant flowing through it. Coolant flows through the loop(s), loses heat to the steam generator, and returns to the core to get warmed up again. Each 'chunk' of water has a temperature and density, and each 'chunk' of core has certain characteristics depending on if it is fuel, construction material, or control rod.

Neutron flux is simulated as well as reactivity and temperature for all components. The real-time aspect comes in through heat-transfer. Basically, each cycle it will calculate reactivity, then change in power, then total power, and then heat generated per fuel chunk. After this, it will look at how much time has elapsed since the beginning of last cycle, and then calculate heat diffusion and flow rate based off this data.

This allows for heat transfer to occur in 'real-time'. If a cycle takes a bit longer to calculate, then slightly more heat will be transferred and the coolant chunks will move a bit further. This allows operators to use the program as a real-time trainer, with real-time responses.

Lastly, certain protective actions can be initiated. You can scram or adjust control rod heights using the command line.
