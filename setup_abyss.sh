
BASE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

mkfile() {
    mkdir -p "$(dirname "$BASE/$1")"
    if [ ! -f "$BASE/$1" ]; then
        echo "// $2" > "$BASE/$1"
    fi
}

echo -e "\e[36mAbyss proje yapisi olusturuluyor...\e[0m"

# ── core ──────────────────────────────────────────────────────────────────────

mkfile "core/CMakeLists.txt"                            "CMake build tanimi"

# Headers
mkfile "core/include/abyss/object_store.h"              "Blob/tree/commit okuma ve yazma"
mkfile "core/include/abyss/index.h"                     "Staging area - hangi dosyalar staged"
mkfile "core/include/abyss/commit.h"                    "Commit objesi olusturma ve parse etme"
mkfile "core/include/abyss/branch.h"                    "Branch olusturma, silme, listeleme"
mkfile "core/include/abyss/repository.h"                "Repo baslatma ve genel repo islemleri"
mkfile "core/include/abyss/diff.h"                      "Myers diff algoritmasi"
mkfile "core/include/abyss/lock.h"                      "Lock/unlock mekanizmasi (.lock dosyalari)"
mkfile "core/include/abyss/recovery.h"                  "Recover ve GC islemleri"
mkfile "core/include/abyss/utils.h"                     "SHA-256, zlib, dosya IO yardimci fonksiyonlar"
mkfile "core/include/abyss/types.h"                     "Ortak veri tipleri ve enum'lar"

# Sources
mkfile "core/src/main.cpp"                              "CLI giris noktasi - komutlari yakalar ve yonlendirir"
mkfile "core/src/object_store.cpp"                      "write_object / read_object implementasyonu"
mkfile "core/src/index.cpp"                             "Index yukle, guncelle, kaydet"
mkfile "core/src/commit.cpp"                            "Commit objesi olustur ve parse et"
mkfile "core/src/branch.cpp"                            "refs/heads altindaki branch dosyalarini yonet"
mkfile "core/src/repository.cpp"                        "abyss init - .abyss/ klasor yapisini olustur"
mkfile "core/src/diff.cpp"                              "Myers diff implementasyonu"
mkfile "core/src/lock.cpp"                              "Lock al, birak, stale lock temizle"
mkfile "core/src/recovery.cpp"                          "Orphan tespiti, GC mark&sweep, recover"
mkfile "core/src/utils.cpp"                             "SHA-256 hesaplama, zlib compress/decompress"

# Commands
mkfile "core/src/commands/init.cpp"                     "abyss init komutu"
mkfile "core/src/commands/add.cpp"                      "abyss add <dosya> komutu"
mkfile "core/src/commands/commit_cmd.cpp"               "abyss commit -m komutu"
mkfile "core/src/commands/status.cpp"                   "abyss status komutu"
mkfile "core/src/commands/log.cpp"                      "abyss log komutu"
mkfile "core/src/commands/diff_cmd.cpp"                 "abyss diff komutu"
mkfile "core/src/commands/branch_cmd.cpp"               "abyss branch komutu"
mkfile "core/src/commands/checkout.cpp"                 "abyss checkout komutu"
mkfile "core/src/commands/gc.cpp"                       "abyss gc komutu"
mkfile "core/src/commands/recover.cpp"                  "abyss recover komutu"

# Tests
mkfile "core/tests/test_object_store.cpp"               "Object store unit testleri"
mkfile "core/tests/test_index.cpp"                      "Index unit testleri"
mkfile "core/tests/test_diff.cpp"                       "Diff unit testleri"
mkfile "core/tests/test_commit.cpp"                     "Commit unit testleri"

# ── backend ───────────────────────────────────────────────────────────────────

mkfile "backend/main.py"                                "FastAPI app tanimi ve router kayitlari"
mkfile "backend/core_bridge.py"                         "subprocess ile abyss binary cagiran kopru"
mkfile "backend/requirements.txt"                       "Python bagimliliklar"
mkfile "backend/database/models.py"                     "Supabase tablo modelleri (users, repos)"
mkfile "backend/database/connection.py"                 "Supabase baglanti ayarlari"
mkfile "backend/routers/auth.py"                        "Kayit, giris, token endpoint'leri"
mkfile "backend/routers/repos.py"                       "Repo listele, olustur, sil endpoint'leri"
mkfile "backend/routers/commits.py"                     "Commit gecmisi ve detay endpoint'leri"
mkfile "backend/routers/branches.py"                    "Branch listele ve yonet endpoint'leri"
mkfile "backend/.env.example"                           "Ortam degiskenleri sablonu"

# ── frontend ──────────────────────────────────────────────────────────────────

mkfile "frontend/package.json"                          "npm bagimliliklari"
mkfile "frontend/vite.config.js"                        "Vite build ayarlari"
mkfile "frontend/src/main.jsx"                          "React uygulama giris noktasi"
mkfile "frontend/src/App.jsx"                           "Router ve layout tanimi"

# Pages
mkfile "frontend/src/pages/Dashboard.jsx"               "Ana sayfa - repo listesi"
mkfile "frontend/src/pages/Repo.jsx"                    "Repo detay - dosya gezgini + commit listesi"
mkfile "frontend/src/pages/Commit.jsx"                  "Commit detay - diff viewer"
mkfile "frontend/src/pages/NewRepo.jsx"                 "Yeni repo olusturma formu"
mkfile "frontend/src/pages/Login.jsx"                   "Giris sayfasi"
mkfile "frontend/src/pages/Register.jsx"                "Kayit sayfasi"

# Components
mkfile "frontend/src/components/FileTree.jsx"           "Klasor/dosya agaci komponenti"
mkfile "frontend/src/components/CommitList.jsx"         "Commit gecmisi listesi"
mkfile "frontend/src/components/DiffViewer.jsx"         "Satir bazli diff gosterimi"
mkfile "frontend/src/components/BranchSelector.jsx"     "Branch secme dropdown"
mkfile "frontend/src/components/RepoCard.jsx"           "Dashboard repo karti"
mkfile "frontend/src/components/Navbar.jsx"             "Ust navigasyon cubugu"
mkfile "frontend/src/components/Sidebar.jsx"            "Sol sidebar"

# Hooks
mkfile "frontend/src/hooks/useRepo.js"                  "Repo veri cekme hoku"
mkfile "frontend/src/hooks/useCommits.js"               "Commit listesi hoku"
mkfile "frontend/src/hooks/useAuth.js"                  "Supabase auth hoku"

# API
mkfile "frontend/src/api/repos.js"                      "Repo API istekleri"
mkfile "frontend/src/api/commits.js"                    "Commit API istekleri"
mkfile "frontend/src/api/branches.js"                   "Branch API istekleri"

# Store
mkfile "frontend/src/store/authStore.js"                "Auth state yonetimi"
mkfile "frontend/src/store/repoStore.js"                "Repo state yonetimi"

# ── root ──────────────────────────────────────────────────────────────────────
mkfile "README.md"                                      "Proje dokumantasyonu"
mkfile ".gitignore"                                     "Ignore kurallari"

echo ""
echo -e "\e[32mTamamlandi! Klasor yapisi olusturuldu.\e[0m"
echo ""
echo -e "\e[33mSonraki adim: core/CMakeLists.txt yazilacak\e[0m"
echo ""

# Yapıyı göster
echo -e "\e[36mOluşturulan yapı:\e[0m"
find "$BASE" -not -path "*/\.*" | sort | sed "s|$BASE/||" | awk '
{
    n = split($0, a, "/")
    indent = ""
    for (i = 1; i < n; i++) indent = indent "  "
    print indent "├── " a[n]
}'
