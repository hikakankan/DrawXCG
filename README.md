# DrawXCG

X68000 のグラフィックス機能のテストをすることができます。

## 概要

- コマンドを入力することにより、グラフィックス機能を実行することができるプログラムです。
- C++ で書かれています。
- コンパイル時に切り替えることにより、Windows と X68000 で動作させることができます。
- Windows 用は Visual Studio のプロジェクトになっています。
- X68000 用は make を実行–してください。

---

## 対応プラットフォーム

| プラットフォーム | 状態               | 詳細                                 |
|------------------|--------------------|--------------------------------------|
| Windows          | 対応             | Visual Studio プロジェクト      |
| X68000           | 対応  | make でコンパイル可能  |

---

## 使用方法

### Windows でのコンパイル方法

- Windows 用は Visual Studio のプロジェクトになっています。プロジェクトを開いてビルドしてください。

### X68000 でのコンパイル方法

- X68000 用は make を実行してください。
  - X68000 エミュレーター [XM6 TypeG](http://retropc.net/pi/xm6/index.html)で動作させることができます。
  - Windows で [MSYS2](https://www.msys2.org/)上で C++ クロスコンパイラ[elf2x68k](https://github.com/yunkya2/elf2x68k)でコンパイルできます。
  - DrawXCG.x が作られます。これを editdisk でフロッピーディスクのイメージにして X68000 エミュレーター [XM6 TypeG](http://retropc.net/pi/xm6/index.html)で読み込んで実行してください。

## プログラムの使い方

以下のコマンドを入力してください。コマンドは
コマンド名 数値または文字列...
という形式です。コマンド名は先頭から区別ができるなら省略することができます。

基本的には X68000 では IOCSコールによる描画、Windows ではピクセルごとの描画を行います。X68000ではIOCSコールでできないもので、現在必要なものは描画を行う関数を作成しています。

### drawline x1 y1 x2 y2 color
(x1,y1)から(x2,y2)への直線を描画します。color で色を指定します。

### line x1 y1 x2 y2 color
drawline と同じです。

### drawrectangle left top width height color
左上の座標と幅と高さを指定して長方形を描画します。color で色を指定します。

### box left top width height color
drawrectangle と同じです。

### drawellipse left top width height color
左上の座標と幅と高さを指定して楕円を描画します。color で色を指定します。

### drawarc left top width height start_angle sweep_angle color
楕円の一部の弧を描画します。開始角度と角度の幅を指定します。

### drawpie left top width height start_angle sweep_angle color
楕円の扇型を描画します。

### drawroundedrectangle  left top width height x_radius y_radius color
左上の座標と幅と高さ、角の楕円の半径(x方向・y方向)を指定して角が丸い長方形を描画します。color で色を指定します。

### fillrectangle left top width height color
左上の座標と幅と高さを指定して塗りつぶした長方形を描画します。color で色を指定します。

### fill left top width height color
fillrectangle と同じです。

### fillellipse left top width height color
左上の座標と幅と高さを指定して塗りつぶした楕円を描画します。color で色を指定します。

### fillroundedrectangle  left top width height x_radius y_radius color
左上の座標と幅と高さ、角の楕円の半径(x方向・y方向)を指定して塗りつぶした角が丸い長方形を描画します。color で色を指定します。

### filltriangle x1 y1 x2 y2 x3 y3 color
3頂点を指定して塗りつぶした三角形を描画します。color で色を指定します。

### filltrapezoid x1 y1 x2 y2 width1 width2 color
台形の斜辺の両端の座標と上底と下底の幅を指定して塗りつぶした台形を描画します。color で色を指定します。

### crtmod mode
CRT モードを設定します。

### g_clr_on
90 グラフィック表示モードにする
  _G_CLR_ON
  引数: なし
  返り値: D0 が破壊される

### gpalet color color_code
グラフィック・パレットを設定します。

    color: パレットコード
    color_code: カラーコード

### screen sc1 sc2 sc3
画面の設定をします。

    sc1: 表示画面サイズ
      0: 256×256
      1: 512×512
      2: 768×512
    sc2: グラフィック画面の実画面サイズ及び色モード
      0: 1024×1024、16色(グラフィックページ0)
      1: 512×512、16色(グラフィックページ0～3)
      2: 512×512、256色(グラフィックページ0、1)
      3: 512×512、65536色(グラフィックページ0)
    sc3: ディスプレイ解像度(省略可能)
      0: Low（標準解像度）
      1: High（高解像度）

### circle x y r color start_angle end_engle ratio
中心 (x, y)、半径 r、縦横の比率 ratio の楕円の円弧を描画します。color で色を指定します。start_angle、end_engle で円弧開始角度(度)、円弧終了角度(度)を指定します。負の値を指定すると扇型を描きます。ratio は 256 より小さければ横長の楕円、256 より大きければ縦長の楕円、256 のとき円(ただし表示上は円にはなりません)を描きます。

### paint x y color
(x, y)を含む領域を色 color で塗りつぶします。

### pset x y color
点(x, y)を色 color にします。

### point x y
点(x, y)を色を取得します。

### symbol x y str mx my color type d
グラフィック画面の位置(x,y)に横方向の倍率 mx、縦方向の倍率 my で指定されたサイズの文字列 str を表示します。color で色を指定します。type は文字のタイプを指定します。

      0: 全角が12×12ドット
      1: 全角が16×16ドット
      2: 全角が24×24ドット

d は向きを指定します。

      0: 回転しない
      1: 90度回転
      2: 180度回転
      3: 270度回転

### apage page
読み出し、書き込みページを設定します。

### vpage page
表示ページを設定します。

### wipe
画面を消去します。

### window x1 y1 x2 y2
グラフィック画面のウィンドウを設定します。

### drawnumberstring str type x y w h color
グラフィック画面の位置(x,y)にサイズ(w,h)の数値の文字列 str を表示します。str は数字、小数点、マイナスの符号からなる文字列を指定します。color で色を指定します。type は文字の種類を指定します(0 または 1)。

### run
マウス入力を受け付けるようになります。マウス入力領域外をクリックするとマウス入力モードを終了します。(X68000 版では「q」キーを押してもマウス入力モードを終了します)

### exit
終了します。

---

## ライセンス

このプロジェクトは MIT ライセンスのもとで公開されています。

