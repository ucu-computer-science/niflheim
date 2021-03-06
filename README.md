# Niflheim
> Linux Device Driver: [3.2 RPi Display](http://www.lcdwiki.com/3.2inch_RPi_Display) for Raspberry PI


## Tested on:
|     ||
|:----|:-------|
|Hardware| [Raspberry PI 4 Model B](https://www.raspberrypi.com/products/raspberry-pi-4-model-b/)|
|Distribution| [Raspberry Pi OS Lite](https://www.raspberrypi.com/software/operating-systems/)|
|Kernel|[5.10](https://elixir.bootlin.com/linux/v5.10/source)|

## HOW to test
```
git clone git@github.com:m-shiroi/niflheim.git .
```
```bash
cd rpi_3_2
```
```bash
cd display
```
```bash
make all tree && make test
```

Presentations:
- [Version.1](https://docs.google.com/presentation/d/1H2mXxWfIWzL6PnOEkNwOqWe97G-Puh7EYUSbl6d6aRY/edit?usp=sharing)
- [Version.2](https://docs.google.com/presentation/d/1l2MWAab4xdU1kkzGAuMP5_JKcs2w-5u1FLamjRbe9JU/edit?usp=sharing)
- [Final](https://docs.google.com/presentation/d/1o4LpNmQKd-CKkz_3tOk2v4GrwnSacAF7DedF2dG8l6o/edit?usp=sharing)

Demos:
- ![Version.1](media/Version.1.mp4)

## References
- Datasheet -- ![ILI9341](https://cdn-shop.adafruit.com/datasheets/ILI9341.pdf)
