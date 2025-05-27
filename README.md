Brunzit
=======

Brunzit is a C++ application for experimenting with flocks of sonic agents through live coding.  Agents traverse a data terrain, a 2D visualization which can be created from a grey scale image or an audio file. 
Agents make sound based on their position in the data terrain. At the moment, if the terrain is an image, each agent is assigned a sine oscillator, with the frequency controlled by the pixel value. If the terrain is an audio recording, it is segmented and visualized using the [FluCoMa](https://www.flucoma.org/) library. Each agent makes sound by granulating the segment at its position.
For more details, see:

Roma, G. (2025) Live coding sonic flocks. Proceedings of the International Live Coding Conference.


Here are some video examples:

- [Death by flocking](https://vimeo.com/1087989308)
- [Chasing sweet spots](https://vimeo.com/1087990393)


Building
---------
The project is based on the [Cinder](https://libcinder.org/) library. To build using cmake, follow the innstructions here: https://libcinder.org/docs/guides/cmake/cmake.html#building-your-application-with-cmake

