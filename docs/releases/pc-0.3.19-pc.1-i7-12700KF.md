**WSword for PC, built for one CPU: the Intel Core i7-12700KF** (12th gen; it runs on that and newer Intel CPUs like it). Built on **[Savage Wind Recomp](https://github.com/Revivedx/SavageWindRecomp) 0.4.1-alpha.4** (MIT). For everyone else: use the normal releases.

No game files are included. You need your own dump of *Breath of the Wild* **1.9.0** (recommended) or 1.8.2.

### Ray-traced reflections on water

With **Ray-Tracing** on (Settings > Video, or F6 in the game), water now reflects what's around it: the arches and pillars of Zora's Domain in its lakes, cliffs and trees in rivers, all rippling with the water's own waves.

- Each pixel of water sends a ray along the view, reflected off the water's surface (its waves included), against the scene's shapes. Where it meets something that's on screen, the reflection shows that, in its own colours and light.
- How strong it is follows the angle, as real water does: faint looking straight down, strong across a lake.
- Only water gets it: the game marks its water in its own buffers, and WSword reads that mark - nothing else turns into a mirror.
- The sky, and anything out of view or hidden, keep the game's own reflection.
- Like the rest of Ray-Tracing it works where the game has sun shadows (outdoors, in daylight), and costs a little GPU time.

Made without testing in the game. If it's too strong, too weak, or reflects the wrong things, a screenshot with F6 on and off at the same spot is all I need. (`SAVAGEWIND_RT_REFL=<strength>` - 2 by default, 0 none - for trying other strengths.)

### Updating

Unzip `WSword-0.3.19-pc.1-i7-12700KF-windows.zip` **over your WSword folder** and replace the files. Everything else stays as it is, and the game code isn't built again (it's the same as 0.3.18-pc.2's).

### Licences

As in 0.3.17-pc.1: `LICENSE.txt`, `SavageWind-README.md`, `Inter-OFL.txt`, `ChakraPetch-OFL.txt`, `lsfg-vk-LICENSE.md`, `NVIDIA-DLSS-LICENSE.txt`, `ReShade-API-LICENSE.md` and the `streamline` folder's licence. NVIDIA, RTX, DLSS and Reflex are NVIDIA's trademarks; WSword isn't made by or affiliated with NVIDIA, Lossless Scaling's developers or ReShade.
