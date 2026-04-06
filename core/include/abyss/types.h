// Ortak veri tipleri ve enum'lar. Tüm sistemlerde ortak tipler kullanılacak. Ağır dependency olmamasına dikkat edilecek. Enum Class kullanılacak. Type Safety.
// hash tipi string olarak kabul edilebilir ama zayıf bir sistemdir. Binary stringe nazaran daha güçlü. timestamp string olarak kullanılabilir ama parse edilmesi ve
// karşılaştırma yapılması daha maliyetlidir. DiffType'de MODIFIED'da olacak bunu unutma. Exit Codeler biraz daha açıklayıcı olmalı ERROR yerine ERR_NO_REPO tarzı
// açıklayıcı şekilde olmalı. Dosya içeriği çok uzun olmamalı Types.h dosyası fazla şişirilmemeli minimal olmalı.
#include <string>
#include <cstdint>
