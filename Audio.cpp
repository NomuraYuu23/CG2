#include "Audio.h"

//インスタンス
Audio* Audio::GetInstance() {
	static Audio instance;

	return &instance;
}

/// <summary>
/// 初期化
/// </summary>
void Audio::Initialize(const std::string& directoryPath) {

	directoryPath_ = directoryPath;

	HRESULT result;
	IXAudio2MasteringVoice* masterVoice;

	// XAudioのエンジンのインスタンスを生成
	result = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(result));

	// マスターボイスを生成
	result = xAudio2_->CreateMasteringVoice(&masterVoice);
	assert(SUCCEEDED(result));

	indexSoundData_ = 0u;

}

/// <summary>
/// 終了処理
/// </summary>
void Audio::Finalize() {

	// XAudio2解放
	xAudio2_.Reset();
	// 音声データ解放
	for (auto& soundData : soundDatas_) {
		Unload(&soundData);
	}

}

/// <summary>
/// WAV音声読み込み
/// </summary>
/// <param name="filename">WAVファイル名</param>
/// <returns>サウンドデータハンドル</returns>
uint32_t Audio::LoadWave(const std::string& fileName) {

	//サウンドデータが最大数を超えていないか
	assert(indexSoundData_ < kMaxSoundData);
	uint32_t handle = indexSoundData_;
	// 読み込み済みサウンドデータを検索
	auto it = std::find_if(soundDatas_.begin(), soundDatas_.end(), [&](const auto& soundData) {
		return soundData.name == fileName;
	});
	if (it != soundDatas_.end()) {
		// 読み込み済みサウンドデータの要素番号を取得
		handle = static_cast<uint32_t>(std::distance(soundDatas_.begin(), it));
		return handle;
	}

	// ディレクトリパスとファイル名を連結してフルパスを得る
	std::string fullpath = directoryPath_ + fileName;

	HRESULT result = 0;

	// ファイルオープン

	// ファイル入力ストリームのインスタンス
	std::ifstream file;
	// .wavファイルをバイナリモードで開く
	file.open(fileName, std::ios_base::binary);
	// ファイルオープン失敗を検出する
	assert(file.is_open());

	// .wavデータ読み込み

	// RIFFヘッダーの読み込み
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	// ファイルがRIFFがチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	// タイプがWAVEかチェック
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}

	// Formatチャンクの読み込み
	FormatChunk format = {};
	// チャンクヘッダーの確認
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt", 4) != 0) {
		assert(0);
	}

	// チャンク本体の読み込み
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

	// Dataチャンクの読み込み
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	// JUNKチャンクを検出した場合
	if (strncmp(data.id, "JUNK", 4) == 0) {
		// 読み取り位置をJUNKチャンクの終わりまで進める
		file.seekg(data.size, std::ios_base::cur);
		// 再読み込み
		file.read((char*)&data, sizeof(data));
	}
	if (strncmp(data.id, "data", 4) != 0) {
		assert(0);
	}

	// Dataチャンクのデータ部(波形データ)の読み込み
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	// ファイルクローズ

	//Waveファイルを閉じる
	file.close();

	// 読み込んだ音声データをreturn

	// returnする為の音声データ
	SoundData& soundData = soundDatas_.at(handle);

	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;
	soundData.name = fileName;

	return handle;

}

/// <summary>
/// サウンドデータの解放
/// </summary>
/// <param name="soundData">サウンドデータ</param>
void Audio::Unload(SoundData* soundData) {

	// バッファのメモリを解放
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

/// <summary>
/// 音声再生
/// </summary>
/// <param name="soundDataHandle">サウンドデータハンドル</param>
/// <param name="loopFlag">ループ再生フラグ</param>
/// <returns>再生ハンドル</returns>
void Audio::PlayWave(uint32_t soundDataHandle) {

	HRESULT result;

	assert(soundDataHandle <= soundDatas_.size());

	// サウンドデータの参照を取得
	SoundData& soundData = soundDatas_.at(soundDataHandle);

	// 波形フォーマットを元にSourceVoiceの生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	// 再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// 波形データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();

}