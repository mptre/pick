# Maintainer: Calle Erlandsson <calle@thoughtbot.com>
pkgname=pick
pkgver=1.3.0
pkgrel=1
pkgdesc='Fuzzy text selection utility'
arch=('x86_64')
url='https://github.com/thoughtbot/pick/'
license=('MIT')
source=('https://github.com/thoughtbot/pick/releases/download/v1.3.0/pick-1.3.0.tar.gz'
        'https://github.com/thoughtbot/pick/releases/download/v1.3.0/pick-1.3.0.tar.gz.asc')
validpgpkeys=('47B22252BF6914DF964BB50C24735DD835689C84')
sha1sums=('87f7d24ad859bc059f5c5a9bb41f4697e4979bd8'
          'SKIP')

build() {
    cd "$pkgname-$pkgver"
    ./configure --prefix=/usr
    make
}

check() {
    cd "$pkgname-$pkgver"
    make -k check
}

package() {
    cd "$pkgname-$pkgver"
    make DESTDIR="$pkgdir/" install
    install -Dm644 LICENSE "$pkgdir/usr/share/licenses/${pkgname}/LICENSE"
}
