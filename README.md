TextGL
======

This is a software-based opengl subset (which means it's not an
opengl implementation, it just looks a bit like it).

This rendering engine was used in the TMDC winning demo "Litterae Finis".

The GL subset is based on TinyGL by Fabrice Bellard, with a few little
changes;

 - Lots of code I did not need was torn out (like different output
   interfaces)
 - All rendering changed to 32 bit. Not everything is tested, so this
   change may still need work..
 - Mipmap support for textures
 - Non-standard gl extension to poke directly at texture data for
   demo purposes
 - Matrix math changed to use glm
 - Possibly other ugly changes that I don't recall right now
 
Demo
----
The demo source shows how to draw some 3d stuff with the textgl thingy,
as well as how to poke directly at the "bitmap" framebuffer, how to
turn that into ascii art with TextFX7, how to poke directly at the
"text mode" framebuffer, and finally how to dump the result to console
with TextFX7.

All in all, this should be more than enough for you to do some bitchin'
text mode demos.
 
License
------- 

Copyright (c) 2013 Jari Komppa
Based on TinyGL (C) 1997-1998 Fabrice Bellard under ZLib license

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.
   