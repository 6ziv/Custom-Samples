/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include "CubismRenderer_OpenGLES2.hpp"
#include "Math/CubismMatrix44.hpp"
#include "Type/csmVector.hpp"
#include "Model/CubismModel.hpp"
#include <float.h>
#include "openglhelper.hpp"
#ifdef CSM_TARGET_WIN_GL
#include <Windows.h>
#endif

//------------ LIVE2D NAMESPACE ------------
namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

/*********************************************************************************************************************
*                                      CubismClippingManager_OpenGLES2
********************************************************************************************************************/
///< ファイルスコープの変数宣言
namespace {
const csmInt32 ColorChannelCount = 4;   ///< 実験時に1チャンネルの場合は1、RGBだけの場合は3、アルファも含める場合は4
}

CubismClippingManager_OpenGLES2::CubismClippingManager_OpenGLES2() :
                                                                   _currentFrameNo(0)
                                                                   , _clippingMaskBufferSize(256)
{
    CubismRenderer::CubismTextureColor* tmp;
    tmp = CSM_NEW CubismRenderer::CubismTextureColor();
    tmp->R = 1.0f;
    tmp->G = 0.0f;
    tmp->B = 0.0f;
    tmp->A = 0.0f;
    _channelColors.PushBack(tmp);
    tmp = CSM_NEW CubismRenderer::CubismTextureColor();
    tmp->R = 0.0f;
    tmp->G = 1.0f;
    tmp->B = 0.0f;
    tmp->A = 0.0f;
    _channelColors.PushBack(tmp);
    tmp = CSM_NEW CubismRenderer::CubismTextureColor();
    tmp->R = 0.0f;
    tmp->G = 0.0f;
    tmp->B = 1.0f;
    tmp->A = 0.0f;
    _channelColors.PushBack(tmp);
    tmp = CSM_NEW CubismRenderer::CubismTextureColor();
    tmp->R = 0.0f;
    tmp->G = 0.0f;
    tmp->B = 0.0f;
    tmp->A = 1.0f;
    _channelColors.PushBack(tmp);

}

CubismClippingManager_OpenGLES2::~CubismClippingManager_OpenGLES2()
{
    for (csmUint32 i = 0; i < _clippingContextListForMask.GetSize(); i++)
    {
        if (_clippingContextListForMask[i]) CSM_DELETE_SELF(CubismClippingContext, _clippingContextListForMask[i]);
        _clippingContextListForMask[i] = NULL;
    }

    // _clippingContextListForDrawは_clippingContextListForMaskにあるインスタンスを指している。上記の処理により要素ごとのDELETEは不要。
    for (csmUint32 i = 0; i < _clippingContextListForDraw.GetSize(); i++)
    {
        _clippingContextListForDraw[i] = NULL;
    }

    for (csmUint32 i = 0; i < _channelColors.GetSize(); i++)
    {
        if (_channelColors[i]) CSM_DELETE(_channelColors[i]);
        _channelColors[i] = NULL;
    }
}

void CubismClippingManager_OpenGLES2::Initialize(CubismModel& model, csmInt32 drawableCount, const csmInt32** drawableMasks, const csmInt32* drawableMaskCounts)
{
    //クリッピングマスクを使う描画オブジェクトを全て登録する
    //クリッピングマスクは、通常数個程度に限定して使うものとする
    for (csmInt32 i = 0; i < drawableCount; i++)
    {
        if (drawableMaskCounts[i] <= 0)
        {
            //クリッピングマスクが使用されていないアートメッシュ（多くの場合使用しない）
            _clippingContextListForDraw.PushBack(NULL);
            continue;
        }

        // 既にあるClipContextと同じかチェックする
        CubismClippingContext* cc = FindSameClip(drawableMasks[i], drawableMaskCounts[i]);
        if (cc == NULL)
        {
            // 同一のマスクが存在していない場合は生成する
            cc = CSM_NEW CubismClippingContext(this, drawableMasks[i], drawableMaskCounts[i]);
            _clippingContextListForMask.PushBack(cc);
        }

        cc->AddClippedDrawable(i);

        _clippingContextListForDraw.PushBack(cc);
    }
}

CubismClippingContext* CubismClippingManager_OpenGLES2::FindSameClip(const csmInt32* drawableMasks, csmInt32 drawableMaskCounts) const
{
    // 作成済みClippingContextと一致するか確認
    for (csmUint32 i = 0; i < _clippingContextListForMask.GetSize(); i++)
    {
        CubismClippingContext* cc = _clippingContextListForMask[i];
        const csmInt32 count = cc->_clippingIdCount;
        if (count != drawableMaskCounts) continue; //個数が違う場合は別物
        csmInt32 samecount = 0;

        // 同じIDを持つか確認。配列の数が同じなので、一致した個数が同じなら同じ物を持つとする。
        for (csmInt32 j = 0; j < count; j++)
        {
            const csmInt32 clipId = cc->_clippingIdList[j];
            for (csmInt32 k = 0; k < count; k++)
            {
                if (drawableMasks[k] == clipId)
                {
                    samecount++;
                    break;
                }
            }
        }
        if (samecount == count)
        {
            return cc;
        }
    }
    return NULL; //見つからなかった
}

void CubismClippingManager_OpenGLES2::SetupClippingContext(CubismModel& model, CubismRenderer_OpenGLES2* renderer, GLint lastFBO, GLint lastViewport[4])
{
    _currentFrameNo++;

    // 全てのクリッピングを用意する
    // 同じクリップ（複数の場合はまとめて１つのクリップ）を使う場合は１度だけ設定する
    csmInt32 usingClipCount = 0;
    for (csmUint32 clipIndex = 0; clipIndex < _clippingContextListForMask.GetSize(); clipIndex++)
    {
        // １つのクリッピングマスクに関して
        CubismClippingContext* cc = _clippingContextListForMask[clipIndex];

        // このクリップを利用する描画オブジェクト群全体を囲む矩形を計算
        CalcClippedDrawTotalBounds(model, cc);

        if (cc->_isUsing)
        {
            usingClipCount++; //使用中としてカウント
        }
    }

    // マスク作成処理
    if (usingClipCount > 0)
    {
        if (!renderer->IsUsingHighPrecisionMask())
        {
            // 生成したFrameBufferと同じサイズでビューポートを設定
            OpenGLHelper::get()->glViewport(0, 0, _clippingMaskBufferSize, _clippingMaskBufferSize);

            // モデル描画時にDrawMeshNowに渡される変換（モデルtoワールド座標変換）
            CubismMatrix44 modelToWorldF = renderer->GetMvpMatrix();

            renderer->PreDraw(); // バッファをクリアする

            // _offscreenFrameBufferへ切り替え
            renderer->_offscreenFrameBuffer.BeginDraw(lastFBO);

            // マスクをクリアする
            // 1が無効（描かれない）領域、0が有効（描かれる）領域。（シェーダで Cd*Csで0に近い値をかけてマスクを作る。1をかけると何も起こらない）
            renderer->_offscreenFrameBuffer.Clear(1.0f, 1.0f, 1.0f, 1.0f);
        }

        // 各マスクのレイアウトを決定していく
        SetupLayoutBounds(renderer->IsUsingHighPrecisionMask() ? 0 : usingClipCount);

        // 実際にマスクを生成する
        // 全てのマスクをどの様にレイアウトして描くかを決定し、ClipContext , ClippedDrawContext に記憶する
        for (csmUint32 clipIndex = 0; clipIndex < _clippingContextListForMask.GetSize(); clipIndex++)
        {
            // --- 実際に１つのマスクを描く ---
            CubismClippingContext* clipContext = _clippingContextListForMask[clipIndex];
            csmRectF* allClippedDrawRect = clipContext->_allClippedDrawRect; //このマスクを使う、全ての描画オブジェクトの論理座標上の囲み矩形
            csmRectF* layoutBoundsOnTex01 = clipContext->_layoutBounds; //この中にマスクを収める

            // モデル座標上の矩形を、適宜マージンを付けて使う
            const csmFloat32 MARGIN = 0.05f;
            _tmpBoundsOnModel.SetRect(allClippedDrawRect);
            _tmpBoundsOnModel.Expand(allClippedDrawRect->Width * MARGIN, allClippedDrawRect->Height * MARGIN);
            //########## 本来は割り当てられた領域の全体を使わず必要最低限のサイズがよい

            // シェーダ用の計算式を求める。回転を考慮しない場合は以下のとおり
            // movePeriod' = movePeriod * scaleX + offX     [[ movePeriod' = (movePeriod - tmpBoundsOnModel.movePeriod)*scale + layoutBoundsOnTex01.movePeriod ]]
            const csmFloat32 scaleX = layoutBoundsOnTex01->Width / _tmpBoundsOnModel.Width;
            const csmFloat32 scaleY = layoutBoundsOnTex01->Height / _tmpBoundsOnModel.Height;

            // マスク生成時に使う行列を求める
            {
                // シェーダに渡す行列を求める <<<<<<<<<<<<<<<<<<<<<<<< 要最適化（逆順に計算すればシンプルにできる）
                _tmpMatrix.LoadIdentity();
                {
                    // Layout0..1 を -1..1に変換
                    _tmpMatrix.TranslateRelative(-1.0f, -1.0f);
                    _tmpMatrix.ScaleRelative(2.0f, 2.0f);
                }
                {
                    // view to Layout0..1
                    _tmpMatrix.TranslateRelative(layoutBoundsOnTex01->X, layoutBoundsOnTex01->Y); //new = [translate]
                    _tmpMatrix.ScaleRelative(scaleX, scaleY); //new = [translate][scale]
                    _tmpMatrix.TranslateRelative(-_tmpBoundsOnModel.X, -_tmpBoundsOnModel.Y);
                    //new = [translate][scale][translate]
                }
                // tmpMatrixForMask が計算結果
                _tmpMatrixForMask.SetMatrix(_tmpMatrix.GetArray());
            }

            //--------- draw時の mask 参照用行列を計算
            {
                // シェーダに渡す行列を求める <<<<<<<<<<<<<<<<<<<<<<<< 要最適化（逆順に計算すればシンプルにできる）
                _tmpMatrix.LoadIdentity();
                {
                    _tmpMatrix.TranslateRelative(layoutBoundsOnTex01->X, layoutBoundsOnTex01->Y); //new = [translate]
                    _tmpMatrix.ScaleRelative(scaleX, scaleY); //new = [translate][scale]
                    _tmpMatrix.TranslateRelative(-_tmpBoundsOnModel.X, -_tmpBoundsOnModel.Y);
                    //new = [translate][scale][translate]
                }

                _tmpMatrixForDraw.SetMatrix(_tmpMatrix.GetArray());
            }

            clipContext->_matrixForMask.SetMatrix(_tmpMatrixForMask.GetArray());

            clipContext->_matrixForDraw.SetMatrix(_tmpMatrixForDraw.GetArray());

            if (!renderer->IsUsingHighPrecisionMask())
            {
                const csmInt32 clipDrawCount = clipContext->_clippingIdCount;
                for (csmInt32 i = 0; i < clipDrawCount; i++)
                {
                    const csmInt32 clipDrawIndex = clipContext->_clippingIdList[i];

                    // 頂点情報が更新されておらず、信頼性がない場合は描画をパスする
                    if (!model.GetDrawableDynamicFlagVertexPositionsDidChange(clipDrawIndex))
                    {
                        continue;
                    }

                    renderer->IsCulling(model.GetDrawableCulling(clipDrawIndex) != 0);

                    // 今回専用の変換を適用して描く
                    // チャンネルも切り替える必要がある(A,R,G,B)
                    renderer->SetClippingContextBufferForMask(clipContext);
                    renderer->DrawMesh(
                        model.GetDrawableTextureIndices(clipDrawIndex),
                        model.GetDrawableVertexIndexCount(clipDrawIndex),
                        model.GetDrawableVertexCount(clipDrawIndex),
                        const_cast<csmUint16*>(model.GetDrawableVertexIndices(clipDrawIndex)),
                        const_cast<csmFloat32*>(model.GetDrawableVertices(clipDrawIndex)),
                        reinterpret_cast<csmFloat32*>(const_cast<Core::csmVector2*>(model.GetDrawableVertexUvs(clipDrawIndex))),
                        model.GetDrawableOpacity(clipDrawIndex),
                        CubismRenderer::CubismBlendMode_Normal,   //クリッピングは通常描画を強制
                        false   // マスク生成時はクリッピングの反転使用は全く関係がない
                    );
                }
            }
        }

        if (!renderer->IsUsingHighPrecisionMask())
        {
            // --- 後処理 ---
            renderer->_offscreenFrameBuffer.EndDraw(); // 描画対象を戻す
            renderer->SetClippingContextBufferForMask(NULL);
            OpenGLHelper::get()->glViewport(lastViewport[0], lastViewport[1], lastViewport[2], lastViewport[3]);
        }
    }
}

void CubismClippingManager_OpenGLES2::CalcClippedDrawTotalBounds(CubismModel& model, CubismClippingContext* clippingContext)
{
    // 被クリッピングマスク（マスクされる描画オブジェクト）の全体の矩形
    csmFloat32 clippedDrawTotalMinX = FLT_MAX, clippedDrawTotalMinY = FLT_MAX;
    csmFloat32 clippedDrawTotalMaxX = FLT_MIN, clippedDrawTotalMaxY = FLT_MIN;

    // このマスクが実際に必要か判定する
    // このクリッピングを利用する「描画オブジェクト」がひとつでも使用可能であればマスクを生成する必要がある

    const csmInt32 clippedDrawCount = clippingContext->_clippedDrawableIndexList->GetSize();
    for (csmInt32 clippedDrawableIndex = 0; clippedDrawableIndex < clippedDrawCount; clippedDrawableIndex++)
    {
        // マスクを使用する描画オブジェクトの描画される矩形を求める
        const csmInt32 drawableIndex = (*clippingContext->_clippedDrawableIndexList)[clippedDrawableIndex];

        const csmInt32 drawableVertexCount = model.GetDrawableVertexCount(drawableIndex);
        csmFloat32* drawableVertexes = const_cast<csmFloat32*>(model.GetDrawableVertices(drawableIndex));

        csmFloat32 minX = FLT_MAX, minY = FLT_MAX;
        csmFloat32 maxX = FLT_MIN, maxY = FLT_MIN;

        csmInt32 loop = drawableVertexCount * Constant::VertexStep;
        for (csmInt32 pi = Constant::VertexOffset; pi < loop; pi += Constant::VertexStep)
        {
            csmFloat32 x = drawableVertexes[pi];
            csmFloat32 y = drawableVertexes[pi + 1];
            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;
        }

        //
        if (minX == FLT_MAX) continue; //有効な点がひとつも取れなかったのでスキップする

        // 全体の矩形に反映
        if (minX < clippedDrawTotalMinX) clippedDrawTotalMinX = minX;
        if (minY < clippedDrawTotalMinY) clippedDrawTotalMinY = minY;
        if (maxX > clippedDrawTotalMaxX) clippedDrawTotalMaxX = maxX;
        if (maxY > clippedDrawTotalMaxY) clippedDrawTotalMaxY = maxY;
    }
    if (clippedDrawTotalMinX == FLT_MAX)
    {
        clippingContext->_allClippedDrawRect->X = 0.0f;
        clippingContext->_allClippedDrawRect->Y = 0.0f;
        clippingContext->_allClippedDrawRect->Width = 0.0f;
        clippingContext->_allClippedDrawRect->Height = 0.0f;
        clippingContext->_isUsing = false;
    }
    else
    {
        clippingContext->_isUsing = true;
        csmFloat32 w = clippedDrawTotalMaxX - clippedDrawTotalMinX;
        csmFloat32 h = clippedDrawTotalMaxY - clippedDrawTotalMinY;
        clippingContext->_allClippedDrawRect->X = clippedDrawTotalMinX;
        clippingContext->_allClippedDrawRect->Y = clippedDrawTotalMinY;
        clippingContext->_allClippedDrawRect->Width = w;
        clippingContext->_allClippedDrawRect->Height = h;
    }
}

void CubismClippingManager_OpenGLES2::SetupLayoutBounds(csmInt32 usingClipCount) const
{
    if (usingClipCount <= 0)
    {// この場合は一つのマスクターゲットを毎回クリアして使用する
        for (csmUint32 index = 0; index < _clippingContextListForMask.GetSize(); index++)
        {
            CubismClippingContext* cc = _clippingContextListForMask[index];
            cc->_layoutChannelNo = 0; // どうせ毎回消すので固定で良い
            cc->_layoutBounds->X = 0.0f;
            cc->_layoutBounds->Y = 0.0f;
            cc->_layoutBounds->Width = 1.0f;
            cc->_layoutBounds->Height = 1.0f;
        }
        return;
    }

    // ひとつのRenderTextureを極力いっぱいに使ってマスクをレイアウトする
    // マスクグループの数が4以下ならRGBA各チャンネルに１つずつマスクを配置し、5以上6以下ならRGBAを2,2,1,1と配置する

    // RGBAを順番に使っていく。
    const csmInt32 div = usingClipCount / ColorChannelCount; //１チャンネルに配置する基本のマスク個数
    const csmInt32 mod = usingClipCount % ColorChannelCount; //余り、この番号のチャンネルまでに１つずつ配分する

    // RGBAそれぞれのチャンネルを用意していく(0:R , 1:G , 2:B, 3:A, )
    csmInt32 curClipIndex = 0; //順番に設定していく

    for (csmInt32 channelNo = 0; channelNo < ColorChannelCount; channelNo++)
    {
        // このチャンネルにレイアウトする数
        const csmInt32 layoutCount = div + (channelNo < mod ? 1 : 0);

        // 分割方法を決定する
        if (layoutCount == 0)
        {
            // 何もしない
        }
        else if (layoutCount == 1)
        {
            //全てをそのまま使う
            CubismClippingContext* cc = _clippingContextListForMask[curClipIndex++];
            cc->_layoutChannelNo = channelNo;
            cc->_layoutBounds->X = 0.0f;
            cc->_layoutBounds->Y = 0.0f;
            cc->_layoutBounds->Width = 1.0f;
            cc->_layoutBounds->Height = 1.0f;
        }
        else if (layoutCount == 2)
        {
            for (csmInt32 i = 0; i < layoutCount; i++)
            {
                const csmInt32 xpos = i % 2;

                CubismClippingContext* cc = _clippingContextListForMask[curClipIndex++];
                cc->_layoutChannelNo = channelNo;

                cc->_layoutBounds->X = xpos * 0.5f;
                cc->_layoutBounds->Y = 0.0f;
                cc->_layoutBounds->Width = 0.5f;
                cc->_layoutBounds->Height = 1.0f;
                //UVを2つに分解して使う
            }
        }
        else if (layoutCount <= 4)
        {
            //4分割して使う
            for (csmInt32 i = 0; i < layoutCount; i++)
            {
                const csmInt32 xpos = i % 2;
                const csmInt32 ypos = i / 2;

                CubismClippingContext* cc = _clippingContextListForMask[curClipIndex++];
                cc->_layoutChannelNo = channelNo;

                cc->_layoutBounds->X = xpos * 0.5f;
                cc->_layoutBounds->Y = ypos * 0.5f;
                cc->_layoutBounds->Width = 0.5f;
                cc->_layoutBounds->Height = 0.5f;
            }
        }
        else if (layoutCount <= 9)
        {
            //9分割して使う
            for (csmInt32 i = 0; i < layoutCount; i++)
            {
                const csmInt32 xpos = i % 3;
                const csmInt32 ypos = i / 3;

                CubismClippingContext* cc = _clippingContextListForMask[curClipIndex++];
                cc->_layoutChannelNo = channelNo;

                cc->_layoutBounds->X = xpos / 3.0f;
                cc->_layoutBounds->Y = ypos / 3.0f;
                cc->_layoutBounds->Width = 1.0f / 3.0f;
                cc->_layoutBounds->Height = 1.0f / 3.0f;
            }
        }
        else
        {
            CubismLogError("not supported mask count : %d", layoutCount);

            // 開発モードの場合は停止させる
            CSM_ASSERT(0);

            // 引き続き実行する場合、 SetupShaderProgramでオーバーアクセスが発生するので仕方なく適当に入れておく
            // もちろん描画結果はろくなことにならない
            for (csmInt32 i = 0; i < layoutCount; i++)
            {
                CubismClippingContext* cc = _clippingContextListForMask[curClipIndex++];
                cc->_layoutChannelNo = 0;
                cc->_layoutBounds->X = 0.0f;
                cc->_layoutBounds->Y = 0.0f;
                cc->_layoutBounds->Width = 1.0f;
                cc->_layoutBounds->Height = 1.0f;
            }
        }
    }
}

CubismRenderer::CubismTextureColor* CubismClippingManager_OpenGLES2::GetChannelFlagAsColor(csmInt32 channelNo)
{
    return _channelColors[channelNo];
}

csmVector<CubismClippingContext*>* CubismClippingManager_OpenGLES2::GetClippingContextListForDraw()
{
    return &_clippingContextListForDraw;
}

void CubismClippingManager_OpenGLES2::SetClippingMaskBufferSize(csmInt32 size)
{
    _clippingMaskBufferSize = size;
}

csmInt32 CubismClippingManager_OpenGLES2::GetClippingMaskBufferSize() const
{
    return _clippingMaskBufferSize;
}

/*********************************************************************************************************************
*                                      CubismClippingContext
********************************************************************************************************************/
CubismClippingContext::CubismClippingContext(CubismClippingManager_OpenGLES2* manager, const csmInt32* clippingDrawableIndices, csmInt32 clipCount)
{
    _owner = manager;

    // クリップしている（＝マスク用の）Drawableのインデックスリスト
    _clippingIdList = clippingDrawableIndices;

    // マスクの数
    _clippingIdCount = clipCount;

    _layoutChannelNo = 0;

    _allClippedDrawRect = CSM_NEW csmRectF();
    _layoutBounds = CSM_NEW csmRectF();

    _clippedDrawableIndexList = CSM_NEW csmVector<csmInt32>();
}

CubismClippingContext::~CubismClippingContext()
{
    if (_layoutBounds != NULL)
    {
        CSM_DELETE(_layoutBounds);
        _layoutBounds = NULL;
    }

    if (_allClippedDrawRect != NULL)
    {
        CSM_DELETE(_allClippedDrawRect);
        _allClippedDrawRect = NULL;
    }

    if (_clippedDrawableIndexList != NULL)
    {
        CSM_DELETE(_clippedDrawableIndexList);
        _clippedDrawableIndexList = NULL;
    }
}

void CubismClippingContext::AddClippedDrawable(csmInt32 drawableIndex)
{
    _clippedDrawableIndexList->PushBack(drawableIndex);
}

CubismClippingManager_OpenGLES2* CubismClippingContext::GetClippingManager()
{
    return _owner;
}



/*********************************************************************************************************************
*                                      CubismDrawProfile_OpenGL
********************************************************************************************************************/
void CubismRendererProfile_OpenGLES2::SetGlEnable(GLenum index, GLboolean enabled)
{
    if (enabled == GL_TRUE) OpenGLHelper::get()->glEnable(index);
    else OpenGLHelper::get()->glDisable(index);
}

void CubismRendererProfile_OpenGLES2::SetGlEnableVertexAttribArray(GLuint index, GLint enabled)
{
    if (enabled) OpenGLHelper::get()->glEnableVertexAttribArray(index);
    else OpenGLHelper::get()->glDisableVertexAttribArray(index);
}

void CubismRendererProfile_OpenGLES2::Save()
{
    //-- push state --
    OpenGLHelper::get()->glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &_lastArrayBufferBinding);
    OpenGLHelper::get()->glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &_lastElementArrayBufferBinding);
    OpenGLHelper::get()->glGetIntegerv(GL_CURRENT_PROGRAM, &_lastProgram);

    OpenGLHelper::get()->glGetIntegerv(GL_ACTIVE_TEXTURE, &_lastActiveTexture);
    OpenGLHelper::get()->glActiveTexture(GL_TEXTURE1); //テクスチャユニット1をアクティブに（以後の設定対象とする）
    OpenGLHelper::get()->glGetIntegerv(GL_TEXTURE_BINDING_2D, &_lastTexture1Binding2D);

    OpenGLHelper::get()->glActiveTexture(GL_TEXTURE0); //テクスチャユニット0をアクティブに（以後の設定対象とする）
    OpenGLHelper::get()->glGetIntegerv(GL_TEXTURE_BINDING_2D, &_lastTexture0Binding2D);

    OpenGLHelper::get()->glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &_lastVertexAttribArrayEnabled[0]);
    OpenGLHelper::get()->glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &_lastVertexAttribArrayEnabled[1]);
    OpenGLHelper::get()->glGetVertexAttribiv(2, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &_lastVertexAttribArrayEnabled[2]);
    OpenGLHelper::get()->glGetVertexAttribiv(3, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &_lastVertexAttribArrayEnabled[3]);

    _lastScissorTest = OpenGLHelper::get()->glIsEnabled(GL_SCISSOR_TEST);
    _lastStencilTest = OpenGLHelper::get()->glIsEnabled(GL_STENCIL_TEST);
    _lastDepthTest = OpenGLHelper::get()->glIsEnabled(GL_DEPTH_TEST);
    _lastCullFace = OpenGLHelper::get()->glIsEnabled(GL_CULL_FACE);
    _lastBlend = OpenGLHelper::get()->glIsEnabled(GL_BLEND);

    OpenGLHelper::get()->glGetIntegerv(GL_FRONT_FACE, &_lastFrontFace);

    OpenGLHelper::get()->glGetBooleanv(GL_COLOR_WRITEMASK, _lastColorMask);

    // backup blending
    OpenGLHelper::get()->glGetIntegerv(GL_BLEND_SRC_RGB, &_lastBlending[0]);
    OpenGLHelper::get()->glGetIntegerv(GL_BLEND_DST_RGB, &_lastBlending[1]);
    OpenGLHelper::get()->glGetIntegerv(GL_BLEND_SRC_ALPHA, &_lastBlending[2]);
    OpenGLHelper::get()->glGetIntegerv(GL_BLEND_DST_ALPHA, &_lastBlending[3]);

    // モデル描画直前のFBOとビューポートを保存
    OpenGLHelper::get()->glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_lastFBO);
    OpenGLHelper::get()->glGetIntegerv(GL_VIEWPORT, _lastViewport);

}

void CubismRendererProfile_OpenGLES2::Restore()
{
    OpenGLHelper::get()->glUseProgram(_lastProgram);

    SetGlEnableVertexAttribArray(0, _lastVertexAttribArrayEnabled[0]);
    SetGlEnableVertexAttribArray(1, _lastVertexAttribArrayEnabled[1]);
    SetGlEnableVertexAttribArray(2, _lastVertexAttribArrayEnabled[2]);
    SetGlEnableVertexAttribArray(3, _lastVertexAttribArrayEnabled[3]);

    SetGlEnable(GL_SCISSOR_TEST, _lastScissorTest);
    SetGlEnable(GL_STENCIL_TEST, _lastStencilTest);
    SetGlEnable(GL_DEPTH_TEST, _lastDepthTest);
    SetGlEnable(GL_CULL_FACE, _lastCullFace);
    SetGlEnable(GL_BLEND, _lastBlend);

    OpenGLHelper::get()->glFrontFace(_lastFrontFace);

    OpenGLHelper::get()->glColorMask(_lastColorMask[0], _lastColorMask[1], _lastColorMask[2], _lastColorMask[3]);

    OpenGLHelper::get()->glBindBuffer(GL_ARRAY_BUFFER, _lastArrayBufferBinding); //前にバッファがバインドされていたら破棄する必要がある
    OpenGLHelper::get()->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _lastElementArrayBufferBinding);

    OpenGLHelper::get()->glActiveTexture(GL_TEXTURE1); //テクスチャユニット1を復元
    OpenGLHelper::get()->glBindTexture(GL_TEXTURE_2D, _lastTexture1Binding2D);

    OpenGLHelper::get()->glActiveTexture(GL_TEXTURE0); //テクスチャユニット0を復元
    OpenGLHelper::get()->glBindTexture(GL_TEXTURE_2D, _lastTexture0Binding2D);

    OpenGLHelper::get()->glActiveTexture(_lastActiveTexture);

    // restore blending
    OpenGLHelper::get()->glBlendFuncSeparate(_lastBlending[0], _lastBlending[1], _lastBlending[2], _lastBlending[3]);
}


/*********************************************************************************************************************
*                                       CubismShader_OpenGLES2
********************************************************************************************************************/
namespace {
    const csmInt32 ShaderCount = 19; ///< シェーダの数 = マスク生成用 + (通常 + 加算 + 乗算) * (マスク無 + マスク有 + マスク有反転 + マスク無の乗算済アルファ対応版 + マスク有の乗算済アルファ対応版 + マスク有反転の乗算済アルファ対応版)
    CubismShader_OpenGLES2* s_instance;
}

enum ShaderNames
{
    // SetupMask
    ShaderNames_SetupMask,

    //Normal
    ShaderNames_Normal,
    ShaderNames_NormalMasked,
    ShaderNames_NormalMaskedInverted,
    ShaderNames_NormalPremultipliedAlpha,
    ShaderNames_NormalMaskedPremultipliedAlpha,
    ShaderNames_NormalMaskedInvertedPremultipliedAlpha,

    //Add
    ShaderNames_Add,
    ShaderNames_AddMasked,
    ShaderNames_AddMaskedInverted,
    ShaderNames_AddPremultipliedAlpha,
    ShaderNames_AddMaskedPremultipliedAlpha,
    ShaderNames_AddMaskedPremultipliedAlphaInverted,

    //Mult
    ShaderNames_Mult,
    ShaderNames_MultMasked,
    ShaderNames_MultMaskedInverted,
    ShaderNames_MultPremultipliedAlpha,
    ShaderNames_MultMaskedPremultipliedAlpha,
    ShaderNames_MultMaskedPremultipliedAlphaInverted,
};

void CubismShader_OpenGLES2::ReleaseShaderProgram()
{
    for (csmUint32 i = 0; i < _shaderSets.GetSize(); i++)
    {
        if (_shaderSets[i]->ShaderProgram)
        {
            OpenGLHelper::get()->glDeleteProgram(_shaderSets[i]->ShaderProgram);
            _shaderSets[i]->ShaderProgram = 0;
            CSM_DELETE(_shaderSets[i]);
        }
    }
}

// SetupMask
static const csmChar* VertShaderSrcSetupMask =
#if defined(CSM_TARGET_IPHONE_ES2) || defined(CSM_TARGET_ANDROID_ES2)
        "#version 100\n"
#else
        "#version 120\n"
#endif
        "attribute vec4 a_position;"
        "attribute vec2 a_texCoord;"
        "varying vec2 v_texCoord;"
        "varying vec4 v_myPos;"
        "uniform mat4 u_clipMatrix;"
        "void main()"
        "{"
        "gl_Position = u_clipMatrix * a_position;"
        "v_myPos = u_clipMatrix * a_position;"
        "v_texCoord = a_texCoord;"
        "v_texCoord.y = 1.0 - v_texCoord.y;"
        "}";
static const csmChar* FragShaderSrcSetupMask =
#if defined(CSM_TARGET_IPHONE_ES2) || defined(CSM_TARGET_ANDROID_ES2)
        "#version 100\n"
        "precision mediump float;"
#else
        "#version 120\n"
#endif
        "varying vec2 v_texCoord;"
        "varying vec4 v_myPos;"
        "uniform sampler2D s_texture0;"
        "uniform vec4 u_channelFlag;"
        "uniform vec4 u_baseColor;"
        "void main()"
        "{"
        "float isInside = "
        "  step(u_baseColor.x, v_myPos.x/v_myPos.w)"
        "* step(u_baseColor.y, v_myPos.y/v_myPos.w)"
        "* step(v_myPos.x/v_myPos.w, u_baseColor.z)"
        "* step(v_myPos.y/v_myPos.w, u_baseColor.w);"

        "gl_FragColor = u_channelFlag * texture2D(s_texture0 , v_texCoord).a * isInside;"
        "}";
#if defined(CSM_TARGET_ANDROID_ES2)
static const csmChar* FragShaderSrcSetupMaskTegra =
        "#version 100\n"
        "#extension GL_NV_shader_framebuffer_fetch : enable\n"
        "precision mediump float;"
        "varying vec2 v_texCoord;"
        "varying vec4 v_myPos;"
        "uniform sampler2D s_texture0;"
        "uniform vec4 u_channelFlag;"
        "uniform vec4 u_baseColor;"
        "void main()"
        "{"
        "float isInside = "
        "  step(u_baseColor.x, v_myPos.x/v_myPos.w)"
        "* step(u_baseColor.y, v_myPos.y/v_myPos.w)"
        "* step(v_myPos.x/v_myPos.w, u_baseColor.z)"
        "* step(v_myPos.y/v_myPos.w, u_baseColor.w);"

        "gl_FragColor = u_channelFlag * texture2D(s_texture0 , v_texCoord).a * isInside;"
        "}";
#endif

//----- バーテックスシェーダプログラム -----
// Normal & Add & Mult 共通
static const csmChar* VertShaderSrc =
#if defined(CSM_TARGET_IPHONE_ES2) || defined(CSM_TARGET_ANDROID_ES2)
        "#version 100\n"
#else
        "#version 120\n"
#endif
        "attribute vec4 a_position;" //v.vertex
        "attribute vec2 a_texCoord;" //v.texcoord
        "varying vec2 v_texCoord;" //v2f.texcoord
        "uniform mat4 u_matrix;"
        "void main()"
        "{"
        "gl_Position = u_matrix * a_position;"
        "v_texCoord = a_texCoord;"
        "v_texCoord.y = 1.0 - v_texCoord.y;"
        "}";

// Normal & Add & Mult 共通（クリッピングされたものの描画用）
static const csmChar* VertShaderSrcMasked =
#if defined(CSM_TARGET_IPHONE_ES2) || defined(CSM_TARGET_ANDROID_ES2)
        "#version 100\n"
#else
        "#version 120\n"
#endif
        "attribute vec4 a_position;"
        "attribute vec2 a_texCoord;"
        "varying vec2 v_texCoord;"
        "varying vec4 v_clipPos;"
        "uniform mat4 u_matrix;"
        "uniform mat4 u_clipMatrix;"
        "void main()"
        "{"
        "gl_Position = u_matrix * a_position;"
        "v_clipPos = u_clipMatrix * a_position;"
        "v_texCoord = a_texCoord;"
        "v_texCoord.y = 1.0 - v_texCoord.y;"
        "}";

//----- フラグメントシェーダプログラム -----
// Normal & Add & Mult 共通
static const csmChar* FragShaderSrc =
#if defined(CSM_TARGET_IPHONE_ES2) || defined(CSM_TARGET_ANDROID_ES2)
        "#version 100\n"
        "precision mediump float;"
#else
        "#version 120\n"
#endif
        "varying vec2 v_texCoord;" //v2f.texcoord
        "uniform sampler2D s_texture0;" //_MainTex
        "uniform vec4 u_baseColor;" //v2f.color
        "void main()"
        "{"
        "vec4 color = texture2D(s_texture0 , v_texCoord) * u_baseColor;"
        "gl_FragColor = vec4(color.rgb * color.a,  color.a);"
        "}";
#if defined(CSM_TARGET_ANDROID_ES2)
static const csmChar* FragShaderSrcTegra =
        "#version 100\n"
        "#extension GL_NV_shader_framebuffer_fetch : enable\n"
        "precision mediump float;"
        "varying vec2 v_texCoord;" //v2f.texcoord
        "uniform sampler2D s_texture0;" //_MainTex
        "uniform vec4 u_baseColor;" //v2f.color
        "void main()"
        "{"
        "vec4 color = texture2D(s_texture0 , v_texCoord) * u_baseColor;"
        "gl_FragColor = vec4(color.rgb * color.a,  color.a);"
        "}";
#endif

// Normal & Add & Mult 共通 （PremultipliedAlpha）
static const csmChar* FragShaderSrcPremultipliedAlpha =
#if defined(CSM_TARGET_IPHONE_ES2) || defined(CSM_TARGET_ANDROID_ES2)
        "#version 100\n"
        "precision mediump float;"
#else
        "#version 120\n"
#endif
        "varying vec2 v_texCoord;" //v2f.texcoord
        "uniform sampler2D s_texture0;" //_MainTex
        "uniform vec4 u_baseColor;" //v2f.color
        "void main()"
        "{"
        "gl_FragColor = texture2D(s_texture0 , v_texCoord) * u_baseColor;"
        "}";
#if defined(CSM_TARGET_ANDROID_ES2)
static const csmChar* FragShaderSrcPremultipliedAlphaTegra =
        "#version 100\n"
        "#extension GL_NV_shader_framebuffer_fetch : enable\n"
        "precision mediump float;"
        "varying vec2 v_texCoord;" //v2f.texcoord
        "uniform sampler2D s_texture0;" //_MainTex
        "uniform vec4 u_baseColor;" //v2f.color
        "void main()"
        "{"
        "gl_FragColor = texture2D(s_texture0 , v_texCoord) * u_baseColor;"
        "}";
#endif

// Normal & Add & Mult 共通（クリッピングされたものの描画用）
static const csmChar* FragShaderSrcMask =
#if defined(CSM_TARGET_IPHONE_ES2) || defined(CSM_TARGET_ANDROID_ES2)
        "#version 100\n"
        "precision mediump float;"
#else
        "#version 120\n"
#endif
        "varying vec2 v_texCoord;"
        "varying vec4 v_clipPos;"
        "uniform sampler2D s_texture0;"
        "uniform sampler2D s_texture1;"
        "uniform vec4 u_channelFlag;"
        "uniform vec4 u_baseColor;"
        "void main()"
        "{"
        "vec4 col_formask = texture2D(s_texture0 , v_texCoord) * u_baseColor;"
        "col_formask.rgb = col_formask.rgb  * col_formask.a ;"
        "vec4 clipMask = (1.0 - texture2D(s_texture1, v_clipPos.xy / v_clipPos.w)) * u_channelFlag;"
        "float maskVal = clipMask.r + clipMask.g + clipMask.b + clipMask.a;"
        "col_formask = col_formask * maskVal;"
        "gl_FragColor = col_formask;"
        "}";
#if defined(CSM_TARGET_ANDROID_ES2)
static const csmChar* FragShaderSrcMaskTegra =
        "#version 100\n"
        "#extension GL_NV_shader_framebuffer_fetch : enable\n"
        "precision mediump float;"
        "varying vec2 v_texCoord;"
        "varying vec4 v_clipPos;"
        "uniform sampler2D s_texture0;"
        "uniform sampler2D s_texture1;"
        "uniform vec4 u_channelFlag;"
        "uniform vec4 u_baseColor;"
        "void main()"
        "{"
        "vec4 col_formask = texture2D(s_texture0 , v_texCoord) * u_baseColor;"
        "col_formask.rgb = col_formask.rgb  * col_formask.a ;"
        "vec4 clipMask = (1.0 - texture2D(s_texture1, v_clipPos.xy / v_clipPos.w)) * u_channelFlag;"
        "float maskVal = clipMask.r + clipMask.g + clipMask.b + clipMask.a;"
        "col_formask = col_formask * maskVal;"
        "gl_FragColor = col_formask;"
        "}";
#endif

// Normal & Add & Mult 共通（クリッピングされて反転使用の描画用）
static const csmChar* FragShaderSrcMaskInverted =
#if defined(CSM_TARGET_IPHONE_ES2) || defined(CSM_TARGET_ANDROID_ES2)
        "#version 100\n"
        "precision mediump float;"
#else
        "#version 120\n"
#endif
        "varying vec2 v_texCoord;"
        "varying vec4 v_clipPos;"
        "uniform sampler2D s_texture0;"
        "uniform sampler2D s_texture1;"
        "uniform vec4 u_channelFlag;"
        "uniform vec4 u_baseColor;"
        "void main()"
        "{"
        "vec4 col_formask = texture2D(s_texture0 , v_texCoord) * u_baseColor;"
        "col_formask.rgb = col_formask.rgb  * col_formask.a ;"
        "vec4 clipMask = (1.0 - texture2D(s_texture1, v_clipPos.xy / v_clipPos.w)) * u_channelFlag;"
        "float maskVal = clipMask.r + clipMask.g + clipMask.b + clipMask.a;"
        "col_formask = col_formask * (1.0 - maskVal);"
        "gl_FragColor = col_formask;"
        "}";
#if defined(CSM_TARGET_ANDROID_ES2)
static const csmChar* FragShaderSrcMaskInvertedTegra =
        "#version 100\n"
        "#extension GL_NV_shader_framebuffer_fetch : enable\n"
        "precision mediump float;"
        "varying vec2 v_texCoord;"
        "varying vec4 v_clipPos;"
        "uniform sampler2D s_texture0;"
        "uniform sampler2D s_texture1;"
        "uniform vec4 u_channelFlag;"
        "uniform vec4 u_baseColor;"
        "void main()"
        "{"
        "vec4 col_formask = texture2D(s_texture0 , v_texCoord) * u_baseColor;"
        "col_formask.rgb = col_formask.rgb  * col_formask.a ;"
        "vec4 clipMask = (1.0 - texture2D(s_texture1, v_clipPos.xy / v_clipPos.w)) * u_channelFlag;"
        "float maskVal = clipMask.r + clipMask.g + clipMask.b + clipMask.a;"
        "col_formask = col_formask * (1.0 - maskVal);"
        "gl_FragColor = col_formask;"
        "}";
#endif

// Normal & Add & Mult 共通（クリッピングされたものの描画用、PremultipliedAlphaの場合）
static const csmChar* FragShaderSrcMaskPremultipliedAlpha =
#if defined(CSM_TARGET_IPHONE_ES2) || defined(CSM_TARGET_ANDROID_ES2)
        "#version 100\n"
        "precision mediump float;"
#else
        "#version 120\n"
#endif
        "varying vec2 v_texCoord;"
        "varying vec4 v_clipPos;"
        "uniform sampler2D s_texture0;"
        "uniform sampler2D s_texture1;"
        "uniform vec4 u_channelFlag;"
        "uniform vec4 u_baseColor;"
        "void main()"
        "{"
        "vec4 col_formask = texture2D(s_texture0 , v_texCoord) * u_baseColor;"
        "vec4 clipMask = (1.0 - texture2D(s_texture1, v_clipPos.xy / v_clipPos.w)) * u_channelFlag;"
        "float maskVal = clipMask.r + clipMask.g + clipMask.b + clipMask.a;"
        "col_formask = col_formask * maskVal;"
        "gl_FragColor = col_formask;"
        "}";
#if defined(CSM_TARGET_ANDROID_ES2)
static const csmChar* FragShaderSrcMaskPremultipliedAlphaTegra =
        "#version 100\n"
        "#extension GL_NV_shader_framebuffer_fetch : enable\n"
        "precision mediump float;"
        "varying vec2 v_texCoord;"
        "varying vec4 v_clipPos;"
        "uniform sampler2D s_texture0;"
        "uniform sampler2D s_texture1;"
        "uniform vec4 u_channelFlag;"
        "uniform vec4 u_baseColor;"
        "void main()"
        "{"
        "vec4 col_formask = texture2D(s_texture0 , v_texCoord) * u_baseColor;"
        "vec4 clipMask = (1.0 - texture2D(s_texture1, v_clipPos.xy / v_clipPos.w)) * u_channelFlag;"
        "float maskVal = clipMask.r + clipMask.g + clipMask.b + clipMask.a;"
        "col_formask = col_formask * maskVal;"
        "gl_FragColor = col_formask;"
        "}";
#endif

// Normal & Add & Mult 共通（クリッピングされて反転使用の描画用、PremultipliedAlphaの場合）
static const csmChar* FragShaderSrcMaskInvertedPremultipliedAlpha =
#if defined(CSM_TARGET_IPHONE_ES2) || defined(CSM_TARGET_ANDROID_ES2)
        "#version 100\n"
        "precision mediump float;"
#else
        "#version 120\n"
#endif
        "varying vec2 v_texCoord;"
        "varying vec4 v_clipPos;"
        "uniform sampler2D s_texture0;"
        "uniform sampler2D s_texture1;"
        "uniform vec4 u_channelFlag;"
        "uniform vec4 u_baseColor;"
        "void main()"
        "{"
        "vec4 col_formask = texture2D(s_texture0 , v_texCoord) * u_baseColor;"
        "vec4 clipMask = (1.0 - texture2D(s_texture1, v_clipPos.xy / v_clipPos.w)) * u_channelFlag;"
        "float maskVal = clipMask.r + clipMask.g + clipMask.b + clipMask.a;"
        "col_formask = col_formask * (1.0 - maskVal);"
        "gl_FragColor = col_formask;"
        "}";
#if defined(CSM_TARGET_ANDROID_ES2)
static const csmChar* FragShaderSrcMaskInvertedPremultipliedAlphaTegra =
        "#version 100\n"
        "#extension GL_NV_shader_framebuffer_fetch : enable\n"
        "precision mediump float;"
        "varying vec2 v_texCoord;"
        "varying vec4 v_clipPos;"
        "uniform sampler2D s_texture0;"
        "uniform sampler2D s_texture1;"
        "uniform vec4 u_channelFlag;"
        "uniform vec4 u_baseColor;"
        "void main()"
        "{"
        "vec4 col_formask = texture2D(s_texture0 , v_texCoord) * u_baseColor;"
        "vec4 clipMask = (1.0 - texture2D(s_texture1, v_clipPos.xy / v_clipPos.w)) * u_channelFlag;"
        "float maskVal = clipMask.r + clipMask.g + clipMask.b + clipMask.a;"
        "col_formask = col_formask * (1.0 - maskVal);"
        "gl_FragColor = col_formask;"
        "}";
#endif

CubismShader_OpenGLES2::CubismShader_OpenGLES2()
{ }

CubismShader_OpenGLES2::~CubismShader_OpenGLES2()
{
    ReleaseShaderProgram();
}

CubismShader_OpenGLES2* CubismShader_OpenGLES2::GetInstance()
{
    if (s_instance == NULL)
    {
        s_instance = CSM_NEW CubismShader_OpenGLES2();
    }
    return s_instance;
}

void CubismShader_OpenGLES2::DeleteInstance()
{
    if (s_instance)
    {
        CSM_DELETE_SELF(CubismShader_OpenGLES2, s_instance);
        s_instance = NULL;
    }
}

#ifdef CSM_TARGET_ANDROID_ES2
csmBool CubismShader_OpenGLES2::s_extMode = false;
csmBool CubismShader_OpenGLES2::s_extPAMode = false;
void CubismShader_OpenGLES2::SetExtShaderMode(csmBool extMode, csmBool extPAMode) {
    s_extMode = extMode;
    s_extPAMode = extPAMode;
}
#endif

void CubismShader_OpenGLES2::GenerateShaders()
{
    for (csmInt32 i = 0; i < ShaderCount; i++)
    {
        _shaderSets.PushBack(CSM_NEW CubismShaderSet());
    }

#ifdef CSM_TARGET_ANDROID_ES2
    if (s_extMode)
    {
        _shaderSets[0]->ShaderProgram = LoadShaderProgram(VertShaderSrcSetupMask, FragShaderSrcSetupMaskTegra);

        _shaderSets[1]->ShaderProgram = LoadShaderProgram(VertShaderSrc, FragShaderSrcTegra);
        _shaderSets[2]->ShaderProgram = LoadShaderProgram(VertShaderSrcMasked, FragShaderSrcMaskTegra);
        _shaderSets[3]->ShaderProgram = LoadShaderProgram(VertShaderSrcMasked, FragShaderSrcMaskInvertedTegra);
        _shaderSets[4]->ShaderProgram = LoadShaderProgram(VertShaderSrc, FragShaderSrcPremultipliedAlphaTegra);
        _shaderSets[5]->ShaderProgram = LoadShaderProgram(VertShaderSrcMasked, FragShaderSrcMaskPremultipliedAlphaTegra);
        _shaderSets[6]->ShaderProgram = LoadShaderProgram(VertShaderSrcMasked, FragShaderSrcMaskInvertedPremultipliedAlphaTegra);
    }
    else
    {
        _shaderSets[0]->ShaderProgram = LoadShaderProgram(VertShaderSrcSetupMask, FragShaderSrcSetupMask);

        _shaderSets[1]->ShaderProgram = LoadShaderProgram(VertShaderSrc, FragShaderSrc);
        _shaderSets[2]->ShaderProgram = LoadShaderProgram(VertShaderSrcMasked, FragShaderSrcMask);
        _shaderSets[3]->ShaderProgram = LoadShaderProgram(VertShaderSrcMasked, FragShaderSrcMaskInverted);
        _shaderSets[4]->ShaderProgram = LoadShaderProgram(VertShaderSrc, FragShaderSrcPremultipliedAlpha);
        _shaderSets[5]->ShaderProgram = LoadShaderProgram(VertShaderSrcMasked, FragShaderSrcMaskPremultipliedAlpha);
        _shaderSets[6]->ShaderProgram = LoadShaderProgram(VertShaderSrcMasked, FragShaderSrcMaskInvertedPremultipliedAlpha);
    }

    // 加算も通常と同じシェーダーを利用する
    _shaderSets[7]->ShaderProgram = _shaderSets[1]->ShaderProgram;
    _shaderSets[8]->ShaderProgram = _shaderSets[2]->ShaderProgram;
    _shaderSets[9]->ShaderProgram = _shaderSets[3]->ShaderProgram;
    _shaderSets[10]->ShaderProgram = _shaderSets[4]->ShaderProgram;
    _shaderSets[11]->ShaderProgram = _shaderSets[5]->ShaderProgram;
    _shaderSets[12]->ShaderProgram = _shaderSets[6]->ShaderProgram;

    // 乗算も通常と同じシェーダーを利用する
    _shaderSets[13]->ShaderProgram = _shaderSets[1]->ShaderProgram;
    _shaderSets[14]->ShaderProgram = _shaderSets[2]->ShaderProgram;
    _shaderSets[15]->ShaderProgram = _shaderSets[3]->ShaderProgram;
    _shaderSets[16]->ShaderProgram = _shaderSets[4]->ShaderProgram;
    _shaderSets[17]->ShaderProgram = _shaderSets[5]->ShaderProgram;
    _shaderSets[18]->ShaderProgram = _shaderSets[6]->ShaderProgram;

#else

    _shaderSets[0]->ShaderProgram = LoadShaderProgram(VertShaderSrcSetupMask, FragShaderSrcSetupMask);

    _shaderSets[1]->ShaderProgram = LoadShaderProgram(VertShaderSrc, FragShaderSrc);
    _shaderSets[2]->ShaderProgram = LoadShaderProgram(VertShaderSrcMasked, FragShaderSrcMask);
    _shaderSets[3]->ShaderProgram = LoadShaderProgram(VertShaderSrcMasked, FragShaderSrcMaskInverted);
    _shaderSets[4]->ShaderProgram = LoadShaderProgram(VertShaderSrc, FragShaderSrcPremultipliedAlpha);
    _shaderSets[5]->ShaderProgram = LoadShaderProgram(VertShaderSrcMasked, FragShaderSrcMaskPremultipliedAlpha);
    _shaderSets[6]->ShaderProgram = LoadShaderProgram(VertShaderSrcMasked, FragShaderSrcMaskInvertedPremultipliedAlpha);

    // 加算も通常と同じシェーダーを利用する
    _shaderSets[7]->ShaderProgram = _shaderSets[1]->ShaderProgram;
    _shaderSets[8]->ShaderProgram = _shaderSets[2]->ShaderProgram;
    _shaderSets[9]->ShaderProgram = _shaderSets[3]->ShaderProgram;
    _shaderSets[10]->ShaderProgram = _shaderSets[4]->ShaderProgram;
    _shaderSets[11]->ShaderProgram = _shaderSets[5]->ShaderProgram;
    _shaderSets[12]->ShaderProgram = _shaderSets[6]->ShaderProgram;

    // 乗算も通常と同じシェーダーを利用する
    _shaderSets[13]->ShaderProgram = _shaderSets[1]->ShaderProgram;
    _shaderSets[14]->ShaderProgram = _shaderSets[2]->ShaderProgram;
    _shaderSets[15]->ShaderProgram = _shaderSets[3]->ShaderProgram;
    _shaderSets[16]->ShaderProgram = _shaderSets[4]->ShaderProgram;
    _shaderSets[17]->ShaderProgram = _shaderSets[5]->ShaderProgram;
    _shaderSets[18]->ShaderProgram = _shaderSets[6]->ShaderProgram;
#endif

    // SetupMask
    _shaderSets[0]->AttributePositionLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[0]->ShaderProgram, "a_position");
    _shaderSets[0]->AttributeTexCoordLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[0]->ShaderProgram, "a_texCoord");
    _shaderSets[0]->SamplerTexture0Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[0]->ShaderProgram, "s_texture0");
    _shaderSets[0]->UniformClipMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[0]->ShaderProgram, "u_clipMatrix");
    _shaderSets[0]->UnifromChannelFlagLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[0]->ShaderProgram, "u_channelFlag");
    _shaderSets[0]->UniformBaseColorLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[0]->ShaderProgram, "u_baseColor");

    // 通常
    _shaderSets[1]->AttributePositionLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[1]->ShaderProgram, "a_position");
    _shaderSets[1]->AttributeTexCoordLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[1]->ShaderProgram, "a_texCoord");
    _shaderSets[1]->SamplerTexture0Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[1]->ShaderProgram, "s_texture0");
    _shaderSets[1]->UniformMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[1]->ShaderProgram, "u_matrix");
    _shaderSets[1]->UniformBaseColorLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[1]->ShaderProgram, "u_baseColor");

    // 通常（クリッピング）
    _shaderSets[2]->AttributePositionLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[2]->ShaderProgram, "a_position");
    _shaderSets[2]->AttributeTexCoordLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[2]->ShaderProgram, "a_texCoord");
    _shaderSets[2]->SamplerTexture0Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[2]->ShaderProgram, "s_texture0");
    _shaderSets[2]->SamplerTexture1Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[2]->ShaderProgram, "s_texture1");
    _shaderSets[2]->UniformMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[2]->ShaderProgram, "u_matrix");
    _shaderSets[2]->UniformClipMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[2]->ShaderProgram, "u_clipMatrix");
    _shaderSets[2]->UnifromChannelFlagLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[2]->ShaderProgram, "u_channelFlag");
    _shaderSets[2]->UniformBaseColorLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[2]->ShaderProgram, "u_baseColor");

    // 通常（クリッピング・反転）
    _shaderSets[3]->AttributePositionLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[3]->ShaderProgram, "a_position");
    _shaderSets[3]->AttributeTexCoordLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[3]->ShaderProgram, "a_texCoord");
    _shaderSets[3]->SamplerTexture0Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[3]->ShaderProgram, "s_texture0");
    _shaderSets[3]->SamplerTexture1Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[3]->ShaderProgram, "s_texture1");
    _shaderSets[3]->UniformMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[3]->ShaderProgram, "u_matrix");
    _shaderSets[3]->UniformClipMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[3]->ShaderProgram, "u_clipMatrix");
    _shaderSets[3]->UnifromChannelFlagLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[3]->ShaderProgram, "u_channelFlag");
    _shaderSets[3]->UniformBaseColorLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[3]->ShaderProgram, "u_baseColor");

    // 通常（PremultipliedAlpha）
    _shaderSets[4]->AttributePositionLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[4]->ShaderProgram, "a_position");
    _shaderSets[4]->AttributeTexCoordLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[4]->ShaderProgram, "a_texCoord");
    _shaderSets[4]->SamplerTexture0Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[4]->ShaderProgram, "s_texture0");
    _shaderSets[4]->UniformMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[4]->ShaderProgram, "u_matrix");
    _shaderSets[4]->UniformBaseColorLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[4]->ShaderProgram, "u_baseColor");

    // 通常（クリッピング、PremultipliedAlpha）
    _shaderSets[5]->AttributePositionLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[5]->ShaderProgram, "a_position");
    _shaderSets[5]->AttributeTexCoordLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[5]->ShaderProgram, "a_texCoord");
    _shaderSets[5]->SamplerTexture0Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[5]->ShaderProgram, "s_texture0");
    _shaderSets[5]->SamplerTexture1Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[5]->ShaderProgram, "s_texture1");
    _shaderSets[5]->UniformMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[5]->ShaderProgram, "u_matrix");
    _shaderSets[5]->UniformClipMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[5]->ShaderProgram, "u_clipMatrix");
    _shaderSets[5]->UnifromChannelFlagLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[5]->ShaderProgram, "u_channelFlag");
    _shaderSets[5]->UniformBaseColorLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[5]->ShaderProgram, "u_baseColor");

    // 通常（クリッピング・反転、PremultipliedAlpha）
    _shaderSets[6]->AttributePositionLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[6]->ShaderProgram, "a_position");
    _shaderSets[6]->AttributeTexCoordLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[6]->ShaderProgram, "a_texCoord");
    _shaderSets[6]->SamplerTexture0Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[6]->ShaderProgram, "s_texture0");
    _shaderSets[6]->SamplerTexture1Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[6]->ShaderProgram, "s_texture1");
    _shaderSets[6]->UniformMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[6]->ShaderProgram, "u_matrix");
    _shaderSets[6]->UniformClipMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[6]->ShaderProgram, "u_clipMatrix");
    _shaderSets[6]->UnifromChannelFlagLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[6]->ShaderProgram, "u_channelFlag");
    _shaderSets[6]->UniformBaseColorLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[6]->ShaderProgram, "u_baseColor");

    // 加算
    _shaderSets[7]->AttributePositionLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[7]->ShaderProgram, "a_position");
    _shaderSets[7]->AttributeTexCoordLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[7]->ShaderProgram, "a_texCoord");
    _shaderSets[7]->SamplerTexture0Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[7]->ShaderProgram, "s_texture0");
    _shaderSets[7]->UniformMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[7]->ShaderProgram, "u_matrix");
    _shaderSets[7]->UniformBaseColorLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[7]->ShaderProgram, "u_baseColor");

    // 加算（クリッピング）
    _shaderSets[8]->AttributePositionLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[8]->ShaderProgram, "a_position");
    _shaderSets[8]->AttributeTexCoordLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[8]->ShaderProgram, "a_texCoord");
    _shaderSets[8]->SamplerTexture0Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[8]->ShaderProgram, "s_texture0");
    _shaderSets[8]->SamplerTexture1Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[8]->ShaderProgram, "s_texture1");
    _shaderSets[8]->UniformMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[8]->ShaderProgram, "u_matrix");
    _shaderSets[8]->UniformClipMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[8]->ShaderProgram, "u_clipMatrix");
    _shaderSets[8]->UnifromChannelFlagLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[8]->ShaderProgram, "u_channelFlag");
    _shaderSets[8]->UniformBaseColorLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[8]->ShaderProgram, "u_baseColor");

    // 加算（クリッピング・反転）
    _shaderSets[9]->AttributePositionLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[9]->ShaderProgram, "a_position");
    _shaderSets[9]->AttributeTexCoordLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[9]->ShaderProgram, "a_texCoord");
    _shaderSets[9]->SamplerTexture0Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[9]->ShaderProgram, "s_texture0");
    _shaderSets[9]->SamplerTexture1Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[9]->ShaderProgram, "s_texture1");
    _shaderSets[9]->UniformMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[9]->ShaderProgram, "u_matrix");
    _shaderSets[9]->UniformClipMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[9]->ShaderProgram, "u_clipMatrix");
    _shaderSets[9]->UnifromChannelFlagLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[9]->ShaderProgram, "u_channelFlag");
    _shaderSets[9]->UniformBaseColorLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[9]->ShaderProgram, "u_baseColor");

    // 加算（PremultipliedAlpha）
    _shaderSets[10]->AttributePositionLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[10]->ShaderProgram, "a_position");
    _shaderSets[10]->AttributeTexCoordLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[10]->ShaderProgram, "a_texCoord");
    _shaderSets[10]->SamplerTexture0Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[10]->ShaderProgram, "s_texture0");
    _shaderSets[10]->UniformMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[10]->ShaderProgram, "u_matrix");
    _shaderSets[10]->UniformBaseColorLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[10]->ShaderProgram, "u_baseColor");

    // 加算（クリッピング、PremultipliedAlpha）
    _shaderSets[11]->AttributePositionLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[11]->ShaderProgram, "a_position");
    _shaderSets[11]->AttributeTexCoordLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[11]->ShaderProgram, "a_texCoord");
    _shaderSets[11]->SamplerTexture0Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[11]->ShaderProgram, "s_texture0");
    _shaderSets[11]->SamplerTexture1Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[11]->ShaderProgram, "s_texture1");
    _shaderSets[11]->UniformMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[11]->ShaderProgram, "u_matrix");
    _shaderSets[11]->UniformClipMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[11]->ShaderProgram, "u_clipMatrix");
    _shaderSets[11]->UnifromChannelFlagLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[11]->ShaderProgram, "u_channelFlag");
    _shaderSets[11]->UniformBaseColorLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[11]->ShaderProgram, "u_baseColor");

    // 加算（クリッピング・反転、PremultipliedAlpha）
    _shaderSets[12]->AttributePositionLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[12]->ShaderProgram, "a_position");
    _shaderSets[12]->AttributeTexCoordLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[12]->ShaderProgram, "a_texCoord");
    _shaderSets[12]->SamplerTexture0Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[12]->ShaderProgram, "s_texture0");
    _shaderSets[12]->SamplerTexture1Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[12]->ShaderProgram, "s_texture1");
    _shaderSets[12]->UniformMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[12]->ShaderProgram, "u_matrix");
    _shaderSets[12]->UniformClipMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[12]->ShaderProgram, "u_clipMatrix");
    _shaderSets[12]->UnifromChannelFlagLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[12]->ShaderProgram, "u_channelFlag");
    _shaderSets[12]->UniformBaseColorLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[12]->ShaderProgram, "u_baseColor");

    // 乗算
    _shaderSets[13]->AttributePositionLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[13]->ShaderProgram, "a_position");
    _shaderSets[13]->AttributeTexCoordLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[13]->ShaderProgram, "a_texCoord");
    _shaderSets[13]->SamplerTexture0Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[13]->ShaderProgram, "s_texture0");
    _shaderSets[13]->UniformMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[13]->ShaderProgram, "u_matrix");
    _shaderSets[13]->UniformBaseColorLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[13]->ShaderProgram, "u_baseColor");

    // 乗算（クリッピング）
    _shaderSets[14]->AttributePositionLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[14]->ShaderProgram, "a_position");
    _shaderSets[14]->AttributeTexCoordLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[14]->ShaderProgram, "a_texCoord");
    _shaderSets[14]->SamplerTexture0Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[14]->ShaderProgram, "s_texture0");
    _shaderSets[14]->SamplerTexture1Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[14]->ShaderProgram, "s_texture1");
    _shaderSets[14]->UniformMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[14]->ShaderProgram, "u_matrix");
    _shaderSets[14]->UniformClipMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[14]->ShaderProgram, "u_clipMatrix");
    _shaderSets[14]->UnifromChannelFlagLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[14]->ShaderProgram, "u_channelFlag");
    _shaderSets[14]->UniformBaseColorLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[14]->ShaderProgram, "u_baseColor");

    // 乗算（クリッピング・反転）
    _shaderSets[15]->AttributePositionLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[15]->ShaderProgram, "a_position");
    _shaderSets[15]->AttributeTexCoordLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[15]->ShaderProgram, "a_texCoord");
    _shaderSets[15]->SamplerTexture0Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[15]->ShaderProgram, "s_texture0");
    _shaderSets[15]->SamplerTexture1Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[15]->ShaderProgram, "s_texture1");
    _shaderSets[15]->UniformMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[15]->ShaderProgram, "u_matrix");
    _shaderSets[15]->UniformClipMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[15]->ShaderProgram, "u_clipMatrix");
    _shaderSets[15]->UnifromChannelFlagLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[15]->ShaderProgram, "u_channelFlag");
    _shaderSets[15]->UniformBaseColorLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[15]->ShaderProgram, "u_baseColor");

    // 乗算（PremultipliedAlpha）
    _shaderSets[16]->AttributePositionLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[16]->ShaderProgram, "a_position");
    _shaderSets[16]->AttributeTexCoordLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[16]->ShaderProgram, "a_texCoord");
    _shaderSets[16]->SamplerTexture0Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[16]->ShaderProgram, "s_texture0");
    _shaderSets[16]->UniformMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[16]->ShaderProgram, "u_matrix");
    _shaderSets[16]->UniformBaseColorLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[16]->ShaderProgram, "u_baseColor");

    // 乗算（クリッピング、PremultipliedAlpha）
    _shaderSets[17]->AttributePositionLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[17]->ShaderProgram, "a_position");
    _shaderSets[17]->AttributeTexCoordLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[17]->ShaderProgram, "a_texCoord");
    _shaderSets[17]->SamplerTexture0Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[17]->ShaderProgram, "s_texture0");
    _shaderSets[17]->SamplerTexture1Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[17]->ShaderProgram, "s_texture1");
    _shaderSets[17]->UniformMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[17]->ShaderProgram, "u_matrix");
    _shaderSets[17]->UniformClipMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[17]->ShaderProgram, "u_clipMatrix");
    _shaderSets[17]->UnifromChannelFlagLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[17]->ShaderProgram, "u_channelFlag");
    _shaderSets[17]->UniformBaseColorLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[17]->ShaderProgram, "u_baseColor");

    // 乗算（クリッピング・反転、PremultipliedAlpha）
    _shaderSets[18]->AttributePositionLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[18]->ShaderProgram, "a_position");
    _shaderSets[18]->AttributeTexCoordLocation = OpenGLHelper::get()->glGetAttribLocation(_shaderSets[18]->ShaderProgram, "a_texCoord");
    _shaderSets[18]->SamplerTexture0Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[18]->ShaderProgram, "s_texture0");
    _shaderSets[18]->SamplerTexture1Location = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[18]->ShaderProgram, "s_texture1");
    _shaderSets[18]->UniformMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[18]->ShaderProgram, "u_matrix");
    _shaderSets[18]->UniformClipMatrixLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[18]->ShaderProgram, "u_clipMatrix");
    _shaderSets[18]->UnifromChannelFlagLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[18]->ShaderProgram, "u_channelFlag");
    _shaderSets[18]->UniformBaseColorLocation = OpenGLHelper::get()->glGetUniformLocation(_shaderSets[18]->ShaderProgram, "u_baseColor");
}

void CubismShader_OpenGLES2::SetupShaderProgram(CubismRenderer_OpenGLES2* renderer, GLuint textureId
                                                , csmInt32 vertexCount, csmFloat32* vertexArray
                                                , csmFloat32* uvArray, csmFloat32 opacity
                                                , CubismRenderer::CubismBlendMode colorBlendMode
                                                , CubismRenderer::CubismTextureColor baseColor
                                                , csmBool isPremultipliedAlpha, CubismMatrix44 matrix4x4
                                                , csmBool invertedMask)
{
    if (_shaderSets.GetSize() == 0)
    {
        GenerateShaders();
    }

    // Blending
    csmInt32 SRC_COLOR;
    csmInt32 DST_COLOR;
    csmInt32 SRC_ALPHA;
    csmInt32 DST_ALPHA;

    if (renderer->GetClippingContextBufferForMask() != NULL) // マスク生成時
    {
        CubismShaderSet* shaderSet = _shaderSets[ShaderNames_SetupMask];
        OpenGLHelper::get()->glUseProgram(shaderSet->ShaderProgram);

        //テクスチャ設定
        OpenGLHelper::get()->glActiveTexture(GL_TEXTURE0);
        OpenGLHelper::get()->glBindTexture(GL_TEXTURE_2D, textureId);
        OpenGLHelper::get()->glUniform1i(shaderSet->SamplerTexture0Location, 0);

        // 頂点配列の設定
        OpenGLHelper::get()->glEnableVertexAttribArray(shaderSet->AttributePositionLocation);
        OpenGLHelper::get()->glVertexAttribPointer(shaderSet->AttributePositionLocation, 2, GL_FLOAT, GL_FALSE, sizeof(csmFloat32) * 2, vertexArray);
        // テクスチャ頂点の設定
        OpenGLHelper::get()->glEnableVertexAttribArray(shaderSet->AttributeTexCoordLocation);
        OpenGLHelper::get()->glVertexAttribPointer(shaderSet->AttributeTexCoordLocation, 2, GL_FLOAT, GL_FALSE, sizeof(csmFloat32) * 2, uvArray);

        // チャンネル
        const csmInt32 channelNo = renderer->GetClippingContextBufferForMask()->_layoutChannelNo;
        CubismRenderer::CubismTextureColor* colorChannel = renderer->GetClippingContextBufferForMask()->GetClippingManager()->GetChannelFlagAsColor(channelNo);
        OpenGLHelper::get()->glUniform4f(shaderSet->UnifromChannelFlagLocation, colorChannel->R, colorChannel->G, colorChannel->B, colorChannel->A);

        OpenGLHelper::get()->glUniformMatrix4fv(shaderSet->UniformClipMatrixLocation, 1, GL_FALSE, renderer->GetClippingContextBufferForMask()->_matrixForMask.GetArray());

        csmRectF* rect = renderer->GetClippingContextBufferForMask()->_layoutBounds;

        OpenGLHelper::get()->glUniform4f(shaderSet->UniformBaseColorLocation,
                    rect->X * 2.0f - 1.0f,
                    rect->Y * 2.0f - 1.0f,
                    rect->GetRight() * 2.0f - 1.0f,
                    rect->GetBottom() * 2.0f - 1.0f);

        SRC_COLOR = GL_ZERO;
        DST_COLOR = GL_ONE_MINUS_SRC_COLOR;
        SRC_ALPHA = GL_ZERO;
        DST_ALPHA = GL_ONE_MINUS_SRC_ALPHA;
    }
    else // マスク生成以外の場合
    {
        const csmBool masked = renderer->GetClippingContextBufferForDraw() != NULL;  // この描画オブジェクトはマスク対象か
        const csmInt32 offset = (masked ? ( invertedMask ? 2 : 1 ) : 0) + (isPremultipliedAlpha ? 3 : 0);

        CubismShaderSet* shaderSet;
        switch (colorBlendMode)
        {
        case CubismRenderer::CubismBlendMode_Normal:
        default:
            shaderSet = _shaderSets[ShaderNames_Normal + offset];
            SRC_COLOR = GL_ONE;
            DST_COLOR = GL_ONE_MINUS_SRC_ALPHA;
            SRC_ALPHA = GL_ONE;
            DST_ALPHA = GL_ONE_MINUS_SRC_ALPHA;
            break;

        case CubismRenderer::CubismBlendMode_Additive:
            shaderSet = _shaderSets[ShaderNames_Add + offset];
            SRC_COLOR = GL_ONE;
            DST_COLOR = GL_ONE;
            SRC_ALPHA = GL_ZERO;
            DST_ALPHA = GL_ONE;
            break;

        case CubismRenderer::CubismBlendMode_Multiplicative:
            shaderSet = _shaderSets[ShaderNames_Mult + offset];
            SRC_COLOR = GL_DST_COLOR;
            DST_COLOR = GL_ONE_MINUS_SRC_ALPHA;
            SRC_ALPHA = GL_ZERO;
            DST_ALPHA = GL_ONE;
            break;
        }

        OpenGLHelper::get()->glUseProgram(shaderSet->ShaderProgram);

        // 頂点配列の設定
        OpenGLHelper::get()->glEnableVertexAttribArray(shaderSet->AttributePositionLocation);
        OpenGLHelper::get()->glVertexAttribPointer(shaderSet->AttributePositionLocation, 2, GL_FLOAT, GL_FALSE, sizeof(csmFloat32) * 2, vertexArray);
        // テクスチャ頂点の設定
        OpenGLHelper::get()->glEnableVertexAttribArray(shaderSet->AttributeTexCoordLocation);
        OpenGLHelper::get()->glVertexAttribPointer(shaderSet->AttributeTexCoordLocation, 2, GL_FLOAT, GL_FALSE, sizeof(csmFloat32) * 2, uvArray);

        if (masked)
        {
            OpenGLHelper::get()->glActiveTexture(GL_TEXTURE1);

            // frameBufferに書かれたテクスチャ
            GLuint tex = renderer->_offscreenFrameBuffer.GetColorBuffer();

            OpenGLHelper::get()->glBindTexture(GL_TEXTURE_2D, tex);
            OpenGLHelper::get()->glUniform1i(shaderSet->SamplerTexture1Location, 1);

            // View座標をClippingContextの座標に変換するための行列を設定
            OpenGLHelper::get()->glUniformMatrix4fv(shaderSet->UniformClipMatrixLocation, 1, 0, renderer->GetClippingContextBufferForDraw()->_matrixForDraw.GetArray());

            // 使用するカラーチャンネルを設定
            const csmInt32 channelNo = renderer->GetClippingContextBufferForDraw()->_layoutChannelNo;
            CubismRenderer::CubismTextureColor* colorChannel = renderer->GetClippingContextBufferForDraw()->GetClippingManager()->GetChannelFlagAsColor(channelNo);
            OpenGLHelper::get()->glUniform4f(shaderSet->UnifromChannelFlagLocation, colorChannel->R, colorChannel->G, colorChannel->B, colorChannel->A);
        }

        //テクスチャ設定
        OpenGLHelper::get()->glActiveTexture(GL_TEXTURE0);
        OpenGLHelper::get()->glBindTexture(GL_TEXTURE_2D, textureId);
        OpenGLHelper::get()->glUniform1i(shaderSet->SamplerTexture0Location, 0);

        //座標変換
        OpenGLHelper::get()->glUniformMatrix4fv(shaderSet->UniformMatrixLocation, 1, 0, matrix4x4.GetArray()); //

        OpenGLHelper::get()->glUniform4f(shaderSet->UniformBaseColorLocation, baseColor.R, baseColor.G, baseColor.B, baseColor.A);
    }

    OpenGLHelper::get()->glBlendFuncSeparate(SRC_COLOR, DST_COLOR, SRC_ALPHA, DST_ALPHA);
}

csmBool CubismShader_OpenGLES2::CompileShaderSource(GLuint* outShader, GLenum shaderType, const csmChar* shaderSource)
{
    GLint status;
    const GLchar* source = shaderSource;

    *outShader = OpenGLHelper::get()->glCreateShader(shaderType);
    OpenGLHelper::get()->glShaderSource(*outShader, 1, &source, NULL);
    OpenGLHelper::get()->glCompileShader(*outShader);

    GLint logLength;
    OpenGLHelper::get()->glGetShaderiv(*outShader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar* log = reinterpret_cast<GLchar*>(CSM_MALLOC(logLength));
        OpenGLHelper::get()->glGetShaderInfoLog(*outShader, logLength, &logLength, log);
        CubismLogError("Shader compile log: %s", log);
        CSM_FREE(log);
    }

    OpenGLHelper::get()->glGetShaderiv(*outShader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        OpenGLHelper::get()->glDeleteShader(*outShader);
        return false;
    }

    return true;
}

csmBool CubismShader_OpenGLES2::LinkProgram(GLuint shaderProgram)
{
    GLint status;
    OpenGLHelper::get()->glLinkProgram(shaderProgram);

    GLint logLength;
    OpenGLHelper::get()->glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar* log = reinterpret_cast<GLchar*>(CSM_MALLOC(logLength));
        OpenGLHelper::get()->glGetProgramInfoLog(shaderProgram, logLength, &logLength, log);
        CubismLogError("Program link log: %s", log);
        CSM_FREE(log);
    }

    OpenGLHelper::get()->glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        return false;
    }

    return true;
}

csmBool CubismShader_OpenGLES2::ValidateProgram(GLuint shaderProgram)
{
    GLint logLength, status;

    OpenGLHelper::get()->glValidateProgram(shaderProgram);
    OpenGLHelper::get()->glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar* log = reinterpret_cast<GLchar*>(CSM_MALLOC(logLength));
        OpenGLHelper::get()->glGetProgramInfoLog(shaderProgram, logLength, &logLength, log);
        CubismLogError("Validate program log: %s", log);
        CSM_FREE(log);
    }

    OpenGLHelper::get()->glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &status);
    if (status == GL_FALSE)
    {
        return false;
    }

    return true;
}

GLuint CubismShader_OpenGLES2::LoadShaderProgram(const csmChar* vertShaderSrc, const csmChar* fragShaderSrc)
{
    GLuint vertShader, fragShader;

    // Create shader program.
    GLuint shaderProgram = OpenGLHelper::get()->glCreateProgram();

    if (!CompileShaderSource(&vertShader, GL_VERTEX_SHADER, vertShaderSrc))
    {
        CubismLogError("Vertex shader compile error!");
        return 0;
    }

    // Create and compile fragment shader.
    if (!CompileShaderSource(&fragShader, GL_FRAGMENT_SHADER, fragShaderSrc))
    {
        CubismLogError("Fragment shader compile error!");
        return 0;
    }

    // Attach vertex shader to program.
    OpenGLHelper::get()->glAttachShader(shaderProgram, vertShader);

    // Attach fragment shader to program.
    OpenGLHelper::get()->glAttachShader(shaderProgram, fragShader);

    // Link program.
    if (!LinkProgram(shaderProgram))
    {
        CubismLogError("Failed to link program: %d", shaderProgram);

        if (vertShader)
        {
            OpenGLHelper::get()->glDeleteShader(vertShader);
            vertShader = 0;
        }
        if (fragShader)
        {
            OpenGLHelper::get()->glDeleteShader(fragShader);
            fragShader = 0;
        }
        if (shaderProgram)
        {
            OpenGLHelper::get()->glDeleteProgram(shaderProgram);
            shaderProgram = 0;
        }

        return 0;
    }

    // Release vertex and fragment shaders.
    if (vertShader)
    {
        OpenGLHelper::get()->glDetachShader(shaderProgram, vertShader);
        OpenGLHelper::get()->glDeleteShader(vertShader);
    }

    if (fragShader)
    {
        OpenGLHelper::get()->glDetachShader(shaderProgram, fragShader);
        OpenGLHelper::get()->glDeleteShader(fragShader);
    }

    return shaderProgram;
}


/*********************************************************************************************************************
 *                                      CubismRenderer_OpenGLES2
 ********************************************************************************************************************/

#ifdef CSM_TARGET_ANDROID_ES2
void CubismRenderer_OpenGLES2::SetExtShaderMode(csmBool extMode, csmBool extPAMode)
{
    CubismShader_OpenGLES2::SetExtShaderMode(extMode, extPAMode);
    CubismShader_OpenGLES2::DeleteInstance();
}

void CubismRenderer_OpenGLES2::ReloadShader()
{
    CubismShader_OpenGLES2::DeleteInstance();
}
#endif


#ifdef CSM_TARGET_WIN_GL

namespace {
PFNGLACTIVETEXTUREPROC OpenGLHelper::get()->glActiveTexture;
PFNGLBINDBUFFERPROC OpenGLHelper::get()->glBindBuffer;
PFNGLUSEPROGRAMPROC OpenGLHelper::get()->glUseProgram;
PFNGLUNIFORM1IPROC OpenGLHelper::get()->glUniform1i;
PFNGLGETATTRIBLOCATIONPROC OpenGLHelper::get()->glGetAttribLocation;
PFNGLGETUNIFORMLOCATIONPROC OpenGLHelper::get()->glGetUniformLocation;
PFNGLBLENDFUNCSEPARATEPROC OpenGLHelper::get()->glBlendFuncSeparate;
//PFNGLBINDVERTEXARRAY            OpenGLHelper::get()->glBindVertexArrayOES ;
PFNGLENABLEVERTEXATTRIBARRAYPROC OpenGLHelper::get()->glEnableVertexAttribArray;
PFNGLDISABLEVERTEXATTRIBARRAYPROC OpenGLHelper::get()->glDisableVertexAttribArray;
PFNGLVERTEXATTRIBPOINTERPROC OpenGLHelper::get()->glVertexAttribPointer;
PFNGLUNIFORMMATRIX4FVPROC OpenGLHelper::get()->glUniformMatrix4fv;
PFNGLUNIFORM1FPROC OpenGLHelper::get()->glUniform1f;
PFNGLUNIFORM4FPROC OpenGLHelper::get()->glUniform4f;

PFNGLLINKPROGRAMPROC OpenGLHelper::get()->glLinkProgram;
PFNGLGETPROGRAMIVPROC OpenGLHelper::get()->glGetProgramiv;
PFNGLVALIDATEPROGRAMPROC OpenGLHelper::get()->glValidateProgram;
PFNGLGETPROGRAMINFOLOGPROC OpenGLHelper::get()->glGetProgramInfoLog;

PFNGLCREATEPROGRAMPROC OpenGLHelper::get()->glCreateProgram;
PFNGLDELETEPROGRAMPROC OpenGLHelper::get()->glDeleteProgram;

PFNGLSHADERSOURCEPROC OpenGLHelper::get()->glShaderSource;
PFNGLGETSHADERIVPROC OpenGLHelper::get()->glGetShaderiv;
PFNGLCOMPILESHADERPROC OpenGLHelper::get()->glCompileShader;
PFNGLCREATESHADERPROC OpenGLHelper::get()->glCreateShader;
PFNGLATTACHSHADERPROC OpenGLHelper::get()->glAttachShader;
PFNGLDETACHSHADERPROC OpenGLHelper::get()->glDetachShader;
PFNGLDELETESHADERPROC OpenGLHelper::get()->glDeleteShader;

PFNGLGENFRAMEBUFFERSEXTPROC OpenGLHelper::get()->glGenFramebuffers;
PFNGLGENRENDERBUFFERSEXTPROC OpenGLHelper::get()->glGenRenderbuffers;
PFNGLBINDFRAMEBUFFEREXTPROC OpenGLHelper::get()->glBindFramebuffer;
PFNGLBINDRENDERBUFFEREXTPROC OpenGLHelper::get()->glBindRenderbuffer;
PFNGLDELETERENDERBUFFERSEXTPROC OpenGLHelper::get()->glDeleteRenderbuffers;
PFNGLDELETEFRAMEBUFFERSEXTPROC OpenGLHelper::get()->glDeleteFramebuffers;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC OpenGLHelper::get()->glFramebufferTexture2D;

PFNGLRENDERBUFFERSTORAGEEXTPROC OpenGLHelper::get()->glRenderbufferStorage;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC OpenGLHelper::get()->glFramebufferRenderbuffer;

PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC OpenGLHelper::get()->glCheckFramebufferStatus;

PFNGLGETVERTEXATTRIBIVPROC OpenGLHelper::get()->glGetVertexAttribiv;

csmBool s_isInitializeGlFunctionsSuccess = false;     ///< 初期化が完了したかどうか。trueなら初期化完了
csmBool s_isFirstInitializeGlFunctions = true;        ///< 最初の初期化実行かどうか。trueなら最初の初期化実行
}

void CubismRenderer_OpenGLES2::InitializeGlFunctions()
{
    if (!s_isFirstInitializeGlFunctions) return;

    s_isInitializeGlFunctionsSuccess = true; //失敗した場合にフラグをfalseにする

    OpenGLHelper::get()->glActiveTexture = (PFNGLACTIVETEXTUREPROC)WinGlGetProcAddress("glActiveTexture");
    //最初の初期化処理が成功したら、以降は初期化処理を行わない
    if (glActiveTexture) s_isFirstInitializeGlFunctions = false;
    else return;

    OpenGLHelper::get()->glBindBuffer = (PFNGLBINDBUFFERPROC)WinGlGetProcAddress("glBindBuffer");
    OpenGLHelper::get()->glUseProgram = (PFNGLUSEPROGRAMPROC)WinGlGetProcAddress("glUseProgram");

    OpenGLHelper::get()->glUniform1i = (PFNGLUNIFORM1IPROC)WinGlGetProcAddress("glUniform1i");
    OpenGLHelper::get()->glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)WinGlGetProcAddress("glGetAttribLocation");
    OpenGLHelper::get()->glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)WinGlGetProcAddress("glGetUniformLocation");
    OpenGLHelper::get()->glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)WinGlGetProcAddress("glBlendFuncSeparate");
    OpenGLHelper::get()->glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)WinGlGetProcAddress("glEnableVertexAttribArray");
    OpenGLHelper::get()->glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)WinGlGetProcAddress("glDisableVertexAttribArray");
    OpenGLHelper::get()->glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)WinGlGetProcAddress("glVertexAttribPointer");

    OpenGLHelper::get()->glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)WinGlGetProcAddress("glUniformMatrix4fv");
    OpenGLHelper::get()->glUniform1f = (PFNGLUNIFORM1FPROC)WinGlGetProcAddress("glUniform1f");
    OpenGLHelper::get()->glUniform4f = (PFNGLUNIFORM4FPROC)WinGlGetProcAddress("glUniform4f");
    OpenGLHelper::get()->glLinkProgram = (PFNGLLINKPROGRAMPROC)WinGlGetProcAddress("glLinkProgram");

    OpenGLHelper::get()->glGetProgramiv = (PFNGLGETPROGRAMIVPROC)WinGlGetProcAddress("glGetProgramiv");
    OpenGLHelper::get()->glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)WinGlGetProcAddress("glValidateProgram");

    OpenGLHelper::get()->glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)WinGlGetProcAddress("glGetProgramInfoLog");
    OpenGLHelper::get()->glCreateProgram = (PFNGLCREATEPROGRAMPROC)WinGlGetProcAddress("glCreateProgram");

    OpenGLHelper::get()->glDeleteProgram = (PFNGLDELETEPROGRAMPROC)WinGlGetProcAddress("glDeleteProgram");
    OpenGLHelper::get()->glShaderSource = (PFNGLSHADERSOURCEPROC)WinGlGetProcAddress("glShaderSource");
    OpenGLHelper::get()->glGetShaderiv = (PFNGLGETSHADERIVPROC)WinGlGetProcAddress("glGetShaderiv");
    OpenGLHelper::get()->glCompileShader = (PFNGLCOMPILESHADERPROC)WinGlGetProcAddress("glCompileShader");
    OpenGLHelper::get()->glCreateShader = (PFNGLCREATESHADERPROC)WinGlGetProcAddress("glCreateShader");
    OpenGLHelper::get()->glAttachShader = (PFNGLATTACHSHADERPROC)WinGlGetProcAddress("glAttachShader");
    OpenGLHelper::get()->glDetachShader = (PFNGLDETACHSHADERPROC)WinGlGetProcAddress("glDetachShader");
    OpenGLHelper::get()->glDeleteShader = (PFNGLDELETESHADERPROC)WinGlGetProcAddress("glDeleteShader");

    OpenGLHelper::get()->glGenFramebuffers = (PFNGLGENFRAMEBUFFERSEXTPROC)WinGlGetProcAddress("glGenFramebuffers");
    OpenGLHelper::get()->glGenRenderbuffers = (PFNGLGENRENDERBUFFERSEXTPROC)WinGlGetProcAddress("glGenRenderbuffers");
    OpenGLHelper::get()->glBindFramebuffer = (PFNGLBINDFRAMEBUFFEREXTPROC)WinGlGetProcAddress("glBindFramebuffer");
    OpenGLHelper::get()->glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)WinGlGetProcAddress("glFramebufferTexture2D");
    OpenGLHelper::get()->glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSEXTPROC)WinGlGetProcAddress("glDeleteFramebuffers");
    OpenGLHelper::get()->glBindRenderbuffer = (PFNGLBINDRENDERBUFFEREXTPROC)WinGlGetProcAddress("glBindRenderbuffer");
    OpenGLHelper::get()->glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSEXTPROC)WinGlGetProcAddress("glDeleteRenderbuffers");

    OpenGLHelper::get()->glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEEXTPROC)WinGlGetProcAddress("glRenderbufferStorage");
    OpenGLHelper::get()->glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)WinGlGetProcAddress("glFramebufferRenderbuffer");
    OpenGLHelper::get()->glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)WinGlGetProcAddress("glCheckFramebufferStatus");
    OpenGLHelper::get()->glGetVertexAttribiv = (PFNGLGETVERTEXATTRIBIVPROC)WinGlGetProcAddress("glGetVertexAttribiv");
}

void* CubismRenderer_OpenGLES2::WinGlGetProcAddress(const csmChar* name)
{
    void* ptr = wglGetProcAddress(name);
    if (ptr == NULL)
    {
        CubismLogError("WinGlGetProcAddress() failed :: %s" , name ) ;
        s_isInitializeGlFunctionsSuccess = false;
    }
    return ptr;
}

void CubismRenderer_OpenGLES2::CheckGlError(const csmChar* message)
{
    GLenum errcode = OpenGLHelper::get()->glGetError();
    if (errcode != GL_NO_ERROR)
    {
        CubismLogError("0x%04X(%4d) : %s", errcode, errcode, message);
    }
}

#endif  //CSM_TARGET_WIN_GL

CubismRenderer* CubismRenderer::Create()
{
    return CSM_NEW CubismRenderer_OpenGLES2();
}

void CubismRenderer::StaticRelease()
{
    CubismRenderer_OpenGLES2::DoStaticRelease();
}

CubismRenderer_OpenGLES2::CubismRenderer_OpenGLES2() : _clippingManager(NULL)
                                                     , _clippingContextBufferForMask(NULL)
                                                     , _clippingContextBufferForDraw(NULL)
{
    // テクスチャ対応マップの容量を確保しておく.
    _textures.PrepareCapacity(32, true);
}

CubismRenderer_OpenGLES2::~CubismRenderer_OpenGLES2()
{
    CSM_DELETE_SELF(CubismClippingManager_OpenGLES2, _clippingManager);
}

void CubismRenderer_OpenGLES2::DoStaticRelease()
{
#ifdef CSM_TARGET_WINGL
    s_isInitializeGlFunctionsSuccess = false;     ///< 初期化が完了したかどうか。trueなら初期化完了
    s_isFirstInitializeGlFunctions = true;        ///< 最初の初期化実行かどうか。trueなら最初の初期化実行
#endif
    CubismShader_OpenGLES2::DeleteInstance();
}

void CubismRenderer_OpenGLES2::Initialize(CubismModel* model)
{
    if (model->IsUsingMasking())
    {
        _clippingManager = CSM_NEW CubismClippingManager_OpenGLES2();  //クリッピングマスク・バッファ前処理方式を初期化
        _clippingManager->Initialize(
            *model,
            model->GetDrawableCount(),
            model->GetDrawableMasks(),
            model->GetDrawableMaskCounts()
        );

        _offscreenFrameBuffer.CreateOffscreenFrame(_clippingManager->GetClippingMaskBufferSize(), _clippingManager->GetClippingMaskBufferSize());
    }

    _sortedDrawableIndexList.Resize(model->GetDrawableCount(), 0);

    CubismRenderer::Initialize(model);  //親クラスの処理を呼ぶ
}

void CubismRenderer_OpenGLES2::PreDraw()
{
#ifdef CSM_TARGET_WIN_GL
    if (s_isFirstInitializeGlFunctions) InitializeGlFunctions();
    if (!s_isInitializeGlFunctionsSuccess) return;
#endif

    OpenGLHelper::get()->glDisable(GL_SCISSOR_TEST);
    OpenGLHelper::get()->glDisable(GL_STENCIL_TEST);
    OpenGLHelper::get()->glDisable(GL_DEPTH_TEST);

    OpenGLHelper::get()->glEnable(GL_BLEND);
    OpenGLHelper::get()->glColorMask(1, 1, 1, 1);

#ifdef CSM_TARGET_IPHONE_ES2
    OpenGLHelper::get()->glBindVertexArrayOES(0);
#endif

    OpenGLHelper::get()->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    OpenGLHelper::get()->glBindBuffer(GL_ARRAY_BUFFER, 0); //前にバッファがバインドされていたら破棄する必要がある

    //異方性フィルタリング。プラットフォームのOpenGLによっては未対応の場合があるので、未設定のときは設定しない
    if (GetAnisotropy() > 0.0f)
    {
        for (csmInt32 i = 0; i < _textures.GetSize(); i++)
        {
            OpenGLHelper::get()->glBindTexture(GL_TEXTURE_2D, _textures[i]);
            OpenGLHelper::get()->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, GetAnisotropy());
        }
    }
}


void CubismRenderer_OpenGLES2::DoDrawModel()
{
    //------------ クリッピングマスク・バッファ前処理方式の場合 ------------
    if (_clippingManager != NULL)
    {
        PreDraw();

        // サイズが違う場合はここで作成しなおし
        if (_offscreenFrameBuffer.GetBufferWidth() != static_cast<csmUint32>(_clippingManager->GetClippingMaskBufferSize()) ||
            _offscreenFrameBuffer.GetBufferHeight() != static_cast<csmUint32>(_clippingManager->GetClippingMaskBufferSize()))
        {
            _offscreenFrameBuffer.DestroyOffscreenFrame();
            _offscreenFrameBuffer.CreateOffscreenFrame(
                static_cast<csmUint32>(_clippingManager->GetClippingMaskBufferSize()), static_cast<csmUint32>(_clippingManager->GetClippingMaskBufferSize()));
        }

        _clippingManager->SetupClippingContext(*GetModel(), this, _rendererProfile._lastFBO, _rendererProfile._lastViewport);
    }

    // 上記クリッピング処理内でも一度PreDrawを呼ぶので注意!!
    PreDraw();

    const csmInt32 drawableCount = GetModel()->GetDrawableCount();
    const csmInt32* renderOrder = GetModel()->GetDrawableRenderOrders();

    // インデックスを描画順でソート
    for (csmInt32 i = 0; i < drawableCount; ++i)
    {
        const csmInt32 order = renderOrder[i];
        _sortedDrawableIndexList[order] = i;
    }

    // 描画
    for (csmInt32 i = 0; i < drawableCount; ++i)
    {
        const csmInt32 drawableIndex = _sortedDrawableIndexList[i];

        // Drawableが表示状態でなければ処理をパスする
        if (!GetModel()->GetDrawableDynamicFlagIsVisible(drawableIndex))
        {
            continue;
        }

        // クリッピングマスク
        CubismClippingContext* clipContext = (_clippingManager != NULL)
            ? (*_clippingManager->GetClippingContextListForDraw())[drawableIndex]
            : NULL;

        if (clipContext != NULL && IsUsingHighPrecisionMask()) // マスクを書く必要がある
        {
            if(clipContext->_isUsing) // 書くことになっていた
            {
                // 生成したFrameBufferと同じサイズでビューポートを設定
                OpenGLHelper::get()->glViewport(0, 0, _clippingManager->GetClippingMaskBufferSize(), _clippingManager->GetClippingMaskBufferSize());

                PreDraw(); // バッファをクリアする

                _offscreenFrameBuffer.BeginDraw(_rendererProfile._lastFBO);

                // マスクをクリアする
                // 1が無効（描かれない）領域、0が有効（描かれる）領域。（シェーダで Cd*Csで0に近い値をかけてマスクを作る。1をかけると何も起こらない）
                _offscreenFrameBuffer.Clear(1.0f, 1.0f, 1.0f, 1.0f);
            }

            {
                const csmInt32 clipDrawCount = clipContext->_clippingIdCount;
                for (csmInt32 index = 0; index < clipDrawCount; index++)
                {
                    const csmInt32 clipDrawIndex = clipContext->_clippingIdList[index];

                    // 頂点情報が更新されておらず、信頼性がない場合は描画をパスする
                    if (!GetModel()->GetDrawableDynamicFlagVertexPositionsDidChange(clipDrawIndex))
                    {
                        continue;
                    }

                    IsCulling(GetModel()->GetDrawableCulling(clipDrawIndex) != 0);

                    // 今回専用の変換を適用して描く
                    // チャンネルも切り替える必要がある(A,R,G,B)
                    SetClippingContextBufferForMask(clipContext);
                    DrawMesh(
                        GetModel()->GetDrawableTextureIndices(clipDrawIndex),
                        GetModel()->GetDrawableVertexIndexCount(clipDrawIndex),
                        GetModel()->GetDrawableVertexCount(clipDrawIndex),
                        const_cast<csmUint16*>(GetModel()->GetDrawableVertexIndices(clipDrawIndex)),
                        const_cast<csmFloat32*>(GetModel()->GetDrawableVertices(clipDrawIndex)),
                        reinterpret_cast<csmFloat32*>(const_cast<Core::csmVector2*>(GetModel()->GetDrawableVertexUvs(clipDrawIndex))),
                        GetModel()->GetDrawableOpacity(clipDrawIndex),
                        CubismRenderer::CubismBlendMode_Normal,   //クリッピングは通常描画を強制
                        false // マスク生成時はクリッピングの反転使用は全く関係がない
                    );
                }
            }

            {
                // --- 後処理 ---
                _offscreenFrameBuffer.EndDraw();
                SetClippingContextBufferForMask(NULL);
                OpenGLHelper::get()->glViewport(_rendererProfile._lastViewport[0], _rendererProfile._lastViewport[1], _rendererProfile._lastViewport[2], _rendererProfile._lastViewport[3]);

                PreDraw(); // バッファをクリアする
            }
        }

        // クリッピングマスクをセットする
        SetClippingContextBufferForDraw(clipContext);

        IsCulling(GetModel()->GetDrawableCulling(drawableIndex) != 0);

        DrawMesh(
            GetModel()->GetDrawableTextureIndices(drawableIndex),
            GetModel()->GetDrawableVertexIndexCount(drawableIndex),
            GetModel()->GetDrawableVertexCount(drawableIndex),
            const_cast<csmUint16*>(GetModel()->GetDrawableVertexIndices(drawableIndex)),
            const_cast<csmFloat32*>(GetModel()->GetDrawableVertices(drawableIndex)),
            reinterpret_cast<csmFloat32*>(const_cast<Core::csmVector2*>(GetModel()->GetDrawableVertexUvs(drawableIndex))),
            GetModel()->GetDrawableOpacity(drawableIndex),
            GetModel()->GetDrawableBlendMode(drawableIndex),
            GetModel()->GetDrawableInvertedMask(drawableIndex) // マスクを反転使用するか
        );
    }

    //
    PostDraw();

}

void CubismRenderer_OpenGLES2::DrawMesh(csmInt32 textureNo, csmInt32 indexCount, csmInt32 vertexCount
                                        , csmUint16* indexArray, csmFloat32* vertexArray, csmFloat32* uvArray
                                        , csmFloat32 opacity, CubismBlendMode colorBlendMode, csmBool invertedMask)
{

#ifdef CSM_TARGET_WIN_GL
    if (s_isFirstInitializeGlFunctions) return;  // WindowsプラットフォームではGL命令のバインドを済ませておく必要がある
#endif

#ifndef CSM_DEBUG
    if (_textures[textureNo] == 0) return;    // モデルが参照するテクスチャがバインドされていない場合は描画をスキップする
#endif

    // 裏面描画の有効・無効
    if (IsCulling())
    {
        OpenGLHelper::get()->glEnable(GL_CULL_FACE);
    }
    else
    {
        OpenGLHelper::get()->glDisable(GL_CULL_FACE);
    }

    OpenGLHelper::get()->glFrontFace(GL_CCW);    // Cubism SDK OpenGLはマスク・アートメッシュ共にCCWが表面

    CubismTextureColor modelColorRGBA = GetModelColor();

    if (GetClippingContextBufferForMask() == NULL) // マスク生成時以外
    {
        modelColorRGBA.A *= opacity;
        if (IsPremultipliedAlpha())
        {
            modelColorRGBA.R *= modelColorRGBA.A;
            modelColorRGBA.G *= modelColorRGBA.A;
            modelColorRGBA.B *= modelColorRGBA.A;
        }
    }

    GLuint drawTextureId;   // シェーダに渡すテクスチャID

    // テクスチャマップからバインド済みテクスチャIDを取得
    // バインドされていなければダミーのテクスチャIDをセットする
    if(_textures[textureNo] != 0)
    {
        drawTextureId = _textures[textureNo];
    }
    else
    {
        drawTextureId = -1;
    }

    CubismShader_OpenGLES2::GetInstance()->SetupShaderProgram(
        this, drawTextureId, vertexCount, vertexArray, uvArray
        , opacity, colorBlendMode, modelColorRGBA, IsPremultipliedAlpha()
        , GetMvpMatrix(), invertedMask
    );

    // ポリゴンメッシュを描画する
    OpenGLHelper::get()->glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, indexArray);

    // 後処理
    OpenGLHelper::get()->glUseProgram(0);
    SetClippingContextBufferForDraw(NULL);
    SetClippingContextBufferForMask(NULL);
}

void CubismRenderer_OpenGLES2::SaveProfile()
{
    _rendererProfile.Save();
}

void CubismRenderer_OpenGLES2::RestoreProfile()
{
    _rendererProfile.Restore();
}

void CubismRenderer_OpenGLES2::BindTexture(csmUint32 modelTextureNo, GLuint glTextureNo)
{
    _textures[modelTextureNo] = glTextureNo;
}

const csmMap<csmInt32, GLuint>& CubismRenderer_OpenGLES2::GetBindedTextures() const
{
    return _textures;
}

void CubismRenderer_OpenGLES2::SetClippingMaskBufferSize(csmInt32 size)
{
    //FrameBufferのサイズを変更するためにインスタンスを破棄・再作成する
    CSM_DELETE_SELF(CubismClippingManager_OpenGLES2, _clippingManager);

    _clippingManager = CSM_NEW CubismClippingManager_OpenGLES2();

    _clippingManager->SetClippingMaskBufferSize(size);

    _clippingManager->Initialize(
        *GetModel(),
        GetModel()->GetDrawableCount(),
        GetModel()->GetDrawableMasks(),
        GetModel()->GetDrawableMaskCounts()
    );
}

csmInt32 CubismRenderer_OpenGLES2::GetClippingMaskBufferSize() const
{
    return _clippingManager->GetClippingMaskBufferSize();
}

void CubismRenderer_OpenGLES2::SetClippingContextBufferForMask(CubismClippingContext* clip)
{
    _clippingContextBufferForMask = clip;
}

CubismClippingContext* CubismRenderer_OpenGLES2::GetClippingContextBufferForMask() const
{
    return _clippingContextBufferForMask;
}

void CubismRenderer_OpenGLES2::SetClippingContextBufferForDraw(CubismClippingContext* clip)
{
    _clippingContextBufferForDraw = clip;
}

CubismClippingContext* CubismRenderer_OpenGLES2::GetClippingContextBufferForDraw() const
{
    return _clippingContextBufferForDraw;
}

}}}}

//------------ LIVE2D NAMESPACE ------------
