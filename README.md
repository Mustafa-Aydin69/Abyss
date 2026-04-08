# Abyss

> Kendi yazdığım versiyon kontrol sistemi ve portfolyo platformu.

Abyss; Git'ten ilham alınarak C++17 ile sıfırdan yazılmış, terminal üzerinden çalışan bir versiyon kontrol sistemidir. Web arayüzü ile birlikte kişisel portfolyo platformu olarak da kullanılmaktadır.

---

## Özellikler

- `abyss init` — repo oluştur
- `abyss add` — dosyaları stage'e al
- `abyss commit` — değişiklikleri kaydet
- `abyss log` — commit geçmişini gör
- `abyss diff` — değişiklikleri karşılaştır
- `abyss branch` / `abyss checkout` — dal yönetimi
- `abyss push` / `abyss pull` — uzak sunucu senkronizasyonu
- Web arayüzü üzerinden repo, commit ve dosya gezgini

---

## Mimari

```
Terminal (CLI)          Tarayıcı (React)
      ↓                       ↓
  C++ Core              FastAPI (köprü)
      ↓                       ↓
  .abyss/            Supabase (metadata)
  objects/
  refs/
  HEAD
```

| Katman     | Teknoloji              |
|------------|------------------------|
| VCS Core   | C++17, zlib, SHA-256   |
| Backend    | Python, FastAPI        |
| Veritabanı | Supabase PostgreSQL    |
| Frontend   | React, Vite            |

---

## Kurulum

### Gereksinimler

```bash
# Debian / Ubuntu
sudo apt install cmake build-essential libssl-dev zlib1g-dev
```

### Derleme

```bash
git clone https://github.com/kullanici/abyss.git
cd abyss/core
mkdir build && cd build
cmake ..
make
sudo make install
```

### Kullanım

```bash
abyss init
abyss add .
abyss commit -m "ilk commit"
abyss log
abyss status
abyss diff
```

---

## Proje Durumu

Aktif geliştirme aşamasında. Agile metodolojisiyle sprint bazlı ilerlenmektedir.

| Sprint | Kapsam                        | Durum       |
|--------|-------------------------------|-------------|
| 1      | Object store, init            | 🔄 Devam ediyor |
| 2      | Index, add, lock              | ⏳ Bekliyor |
| 3      | Commit, log, status           | ⏳ Bekliyor |
| 4      | Diff (Myers)                  | ⏳ Bekliyor |
| 5      | Branch, checkout, merge       | ⏳ Bekliyor |
| 6      | GC, recover, push, pull       | ⏳ Bekliyor |
| 7      | FastAPI backend               | ⏳ Bekliyor |
| 8-9    | React frontend, portfolyo     | ⏳ Bekliyor |
| 10     | Deploy                        | ⏳ Bekliyor |

---

## Lisans

MIT
