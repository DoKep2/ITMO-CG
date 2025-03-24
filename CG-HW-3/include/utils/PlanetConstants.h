namespace PlanetDistanceToSun {
    constexpr float scale = 0.02073f;  // Масштабный коэффициент
    constexpr float sunRadius = 0.5f;  // Радиус Солнца в твоем масштабе

    constexpr float Mercury = 57.9f * scale;  // 1.2
    constexpr float Venus   = 108.2f * scale;
    constexpr float Earth   = 149.6f * scale;
    constexpr float Mars    = 227.9f * scale;
    constexpr float Jupiter = 778.3f * scale;
    constexpr float Saturn  = 1427.0f * scale;
    constexpr float Uranus  = 2871.0f * scale;
    constexpr float Neptune = 4497.1f * scale;
}

namespace PlanetRadius {
    constexpr float scale = 0.00005f;  // Масштаб для нормализации радиусов
    constexpr float Sun     = 0.5f;  // Радиус Солнца в твоем масштабе

    constexpr float Mercury = 2439.7f * scale;
    constexpr float Venus   = 6051.8f * scale;
    constexpr float Earth   = 6371.0f * scale;
    constexpr float Mars    = 3389.5f * scale;
    constexpr float Jupiter = 69911.0f * scale;
    constexpr float Saturn  = 58232.0f * scale;
    constexpr float Uranus  = 25362.0f * scale;
    constexpr float Neptune = 24622.0f * scale;
}

namespace PlanetVelocity {
    constexpr float scale = 0.0002087f;  // Масштаб для нормализации скоростей

    constexpr float Mercury = 47.87f * scale;  // Скорость Меркурия
    constexpr float Venus   = 35.02f * scale;  // Скорость Венеры
    constexpr float Earth   = 29.78f * scale;  // Скорость Земли
    constexpr float Mars    = 24.07f * scale;  // Скорость Марса
    constexpr float Jupiter = 13.07f * scale;  // Скорость Юпитера
    constexpr float Saturn  = 9.69f * scale;   // Скорость Сатурна
    constexpr float Uranus  = 6.81f * scale;   // Скорость Урана
    constexpr float Neptune = 5.43f * scale;   // Скорость Нептуна
}

