# LaneDetection-
CSE396

Bu projeyi CSE396 Proje dersimiz kapsamında yapmış olup Projemizin kodları burada yer almaktadır.


GEREKSİNİMLER

- OpenCV 3.0.0

- Raspberry Pİ 3

- Webcam Camera(Logitech C170) 

- Titreşim motoru

- Direksiyon


Projemiz için kullandığımız yapılar bu şekildedir. Yapmış olduğumuz proje Raspberry Pi 3 üzerinde kamera destekli olarak OpenCV görüntü işleme kütüphanesi kullanılarak gerçekleşmiştir. Görüntü işleme kütüphanemizi kullanarak araç ve şerit tespiti yapılmıştır. Şerit tespiti yapıldıktan sonra buldugumuz seritler Raspberry Pi ekranı üzerinde pembe renkte cizdirilmiştir. Şerit çizdrime dışında ek özellik olarak araç rengini bulma, hız tespiti , trafik yoğunluğu gibi özellikler de sistemimizde mevcuttur.

Sistemimizde kullanıcımızın kullandığı bir direksiyon mevcuttur. Direksiyonumuz ise şeridimiz değiştiğinde kullanıcıya titreşim şeklinde bir uyarı vermektedir. Titreşim için kullandığımız gereksinimler;

- "wiringpi.h" kütüphanesi

GPIO olarak Raspberry Pi 3'ün 12. pinini kullandık.






