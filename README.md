# Pinball

## Description

After adding the pinball background we started working on the hitboxes around the board and the collisions with the pinball objects. While making the hitboxes, we had an issue escalating the pixels and we couldn’t find the positions of the chains. We ended up fixing it when we tried it on another device.

We found another bug with the hitboxes surrounding the board, where the points in the hitboxes chains appeared in random places. We realized that it was because they were outside the map limits.


## Controls

 - 1: Gravity set to -1
 - 2: Gravity set to -2
 - 3: Gravity set to 4
 - 4: Gravity set to -8
 - 5: Horizontal gravity set to -1
 - 6: Horizontal gravity to set 1
 - F1: Debug Mode (Draw shapes + mouse joint) 
 - F2: Toggle draw shapes while mouse joint 
 - N: Toggles stepActive
 - B: If stepActive == False -> do one step


## Developers

 - Oriol Rodríoguez - Coding, Organizer
 - Adrian Ponce - Coding
 - Eric Regalón - Coding
 - Leone Martí - Coding

## License

This project is licensed under an unmodified MIT license, which is an OSI-certified license that allows static linking with closed source software. Check [LICENSE](LICENSE) for further details.

{AdditionalLicenses}
