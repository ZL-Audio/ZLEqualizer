// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <array>

namespace zlInterface::multilingual::ja {
    static constexpr std::array texts = {
        "押す：選択したバンドをオンにします。\n離す：選択したバンドをバイパスします。",
        "押す：選択した周波数帯域に影響を受けるオーディオをソロにします。",
        "選択した周波数帯域のタイプを選択します。",
        "選択した周波数帯域のスロープを選択します。スロープが高いほど、フィルターのレスポンスカーブの変化が急になります。",
        "選択した周波数帯域のステレオモードを選択します。",
        "選択した周波数帯域の中心周波数を調整します。",
        "選択した周波数帯域のゲインを調整します。",
        "選択した周波数帯域のQファクターを調整します。Q値が大きいほど、帯域幅が狭くなります。",
        "左右の矢印で周波数帯域を選択します。",
        "押す：選択した周波数帯域のダイナミック動作をオンにします。",
        "押す：選択した周波数帯域の自動閾値をオンにします。\n離す：選択した周波数帯域の自動閾値をオフにし、学習した閾値を適用します。",
        "クリック：選択した周波数帯域をオフにします。",
        "離す：選択したバンドのダイナミック動作をバイパスします。",
        "押す：選択したバンドのサイドチェーンオーディオをソロにします。",
        "押す：ダイナミック閾値を相対モードに設定します。\n離す：ダイナミック閾値を絶対モードに設定します。",
        "押す：サイドチェーンを反対のステレオモードに設定するためにスワップします。",
        "選択したバンドのダイナミック動作の閾値を調整します。",
        "選択したバンドのダイナミック動作のニー(Knee)幅を調整します。実際のニー幅は表示値 × 60 dBです。",
        "選択したバンドのダイナミック動作のアタックタイムを調整します。",
        "選択したバンドのダイナミック動作のリリースタイムを調整します。",
        "サイドチェーン信号に適用されるバンドパスフィルターの中心周波数を調整します。",
        "サイドチェーン信号に適用されるバンドパスフィルターのQ値を調整します。",
        "押す：外部サイドチェーンを使用します。\n離す：内部サイドチェーンを使用します。",
        "押す：静的ゲイン補正をオンにします。SGCはフィルターのパラメータから補正量を推定します。SGCは不正確ですが、信号のダイナミクスには影響しません。",
        "押す：プラグインをバイパスします。",
        "すべてのゲインパラメータのスケールを調整します。",
        "押す：出力信号の位相を反転します。",
        "押す：自動ゲイン補正をオンにします。AGCは長期的には正確ですが、信号のダイナミクスに影響を与えます。",
        "押す：押すと、入力信号と出力信号の統合ラウドネスの測定を開始します。\n離す：離すと、AGCをオフにし、出力ゲインを2つのラウドネス値の差に更新します。",
        "プラグインの出力ゲインを調整します。",
        "入力スペクトラムアナライザーの状態を選択します。FRZでアナライザーを凍結できます。",
        "出力スペクトラムアナライザーの状態を選択します。FRZでアナライザーを凍結できます。",
        "サイドチェーンスペクトラムアナライザーの状態を選択します。FRZでアナライザーを凍結できます。",
        "スペクトラムアナライザーの減衰速度を調整します。",
        "スペクトラムアナライザーのスロープを調整します（実際の信号には影響しません）。",
        "サイドチェーン信号のルックアヘッド時間を調整します（レイテンシーを導入することで）。",
        "RMSの長さを調整します。RMSが増加すると、アタック/リリースが遅くなり、よりスムーズになります。",
        "リリースの滑らかさを調整します。",
        "高品質オプションの状態を選択します。高品質がオンの場合、ダイナミクスはサンプルごとにフィルター状態を調整し、ダイナミック効果をスムーズにし、アーチファクトを減らします。",
        "衝突検出の状態を選択します。",
        "衝突検出の強度を調整します。強度を上げると、検出される領域が大きくなり、より暗くなります。",
        "衝突検出のスケールを調整します。スケールを上げると、検出される領域がより暗くなります。",
        "フィルターの構造を選択します。",
        "ゼロレイテンシーの状態を選択します。ゼロレイテンシーがオンの場合、追加のレイテンシーはゼロになります。ただし、バッファサイズがダイナミック効果とパラメータオートメーションに若干影響します。",
        "ターゲットカーブを選択します。\nSide：サイドチェーン信号から学習します。\nPreset：プリセットファイルから読み込みます。\nFlat：-4.5 dB/octの平坦な線として設定します。",
        "カーブ学習モデルの重みを調整します。重みが大きいほど、モデルは信号の大きな部分からより多く学習します。",
        "クリック：カーブ学習を開始します。",
        "クリック：現在のターゲットカーブをプリセットファイルとして保存します。",
        "差分カーブの滑らかさを調整します。",
        "差分カーブのスロープを調整します。",
        "フィッティングアルゴリズムを選択します。\nLD：ローカル勾配ベースのアルゴリズム（高速）。\nGN：グローバル勾配フリーアルゴリズム（推奨）。\nGN+：グローバル勾配フリーアルゴリズム（遅い、フィルターがより高いスロープを持つことを許可）。",
        "クリック：カーブフィッティングを開始します。",
        "フィッティングに使用するバンド数を調整します。フィッティングが完了すると、カーブフィッティングモデルがバンド数を提案します。その後、変更することができます。",
        "フィルターレスポンスカーブ（左）とスペクトラムアナライザー（右）のdBスケールを選択します。",
        "クリック：イコライザーマッチパネルを開きます。イコライザーマッチには4つのステップがあります：\n1. ターゲットカーブを選択します。\n2. 学習を開始します。\n3. 差分を調整します。\n4. フィッティングを開始します。",
        "イコライザーの最も一般的なフィルター構造です。フィルターレスポンスはアナログプロトタイプに非常に近いです。変調に適しています。",
        "クロスオーバーの最も一般的なフィルター構造です。フィルターはより多くの位相シフトを引き起こします。高速変調に適しています。",
        "ピーク/シェルフフィルターが並列に配置されています。フィルターレスポンスは表示されるカーブとは異なります。変調に適しています。",
        "追加の補正を伴う最小位相構造です。フィルターレスポンスはアナログプロトタイプとほぼ同一です。変調には適していません。",
        "追加の補正を伴う最小位相構造です。フィルターはアナログプロトタイプの振幅レスポンスを持ち、高域での位相シフトが少なくなっています。変調には適していません。",
        "フィルターはアナログプロトタイプの振幅レスポンスとゼロ位相レスポンスを持ちます。変調には適していません。ダイナミック効果は機能しません。",
        "ダブルクリック：UI設定を開きます。"
    };
}
