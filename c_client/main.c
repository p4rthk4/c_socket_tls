// system lib
#include<stdio.h>
#include <stdio.h>
#include <pthread.h>



// header files
#include <main.h>
#include <client.h>

#define NUM_THREADS 5
#define NUM_FILES 400

const char* file_names[NUM_FILES] = {
        "dummy_file_1.bin", "dummy_file_2.bin", "dummy_file_3.bin", "dummy_file_4.bin", "dummy_file_5.bin",
        "dummy_file_6.bin", "dummy_file_7.bin", "dummy_file_8.bin", "dummy_file_9.bin", "dummy_file_10.bin",
        "dummy_file_11.bin", "dummy_file_12.bin", "dummy_file_13.bin", "dummy_file_14.bin", "dummy_file_15.bin",
        "dummy_file_16.bin", "dummy_file_17.bin", "dummy_file_18.bin", "dummy_file_19.bin", "dummy_file_20.bin",
        "dummy_file_21.bin", "dummy_file_22.bin", "dummy_file_23.bin", "dummy_file_24.bin", "dummy_file_25.bin",
        "dummy_file_26.bin", "dummy_file_27.bin", "dummy_file_28.bin", "dummy_file_29.bin", "dummy_file_30.bin",
        "dummy_file_31.bin", "dummy_file_32.bin", "dummy_file_33.bin", "dummy_file_34.bin", "dummy_file_35.bin",
        "dummy_file_36.bin", "dummy_file_37.bin", "dummy_file_38.bin", "dummy_file_39.bin", "dummy_file_40.bin",
        "dummy_file_41.bin", "dummy_file_42.bin", "dummy_file_43.bin", "dummy_file_44.bin", "dummy_file_45.bin",
        "dummy_file_46.bin", "dummy_file_47.bin", "dummy_file_48.bin", "dummy_file_49.bin", "dummy_file_50.bin",
        "dummy_file_51.bin", "dummy_file_52.bin", "dummy_file_53.bin", "dummy_file_54.bin", "dummy_file_55.bin",
        "dummy_file_56.bin", "dummy_file_57.bin", "dummy_file_58.bin", "dummy_file_59.bin", "dummy_file_60.bin",
        "dummy_file_61.bin", "dummy_file_62.bin", "dummy_file_63.bin", "dummy_file_64.bin", "dummy_file_65.bin",
        "dummy_file_66.bin", "dummy_file_67.bin", "dummy_file_68.bin", "dummy_file_69.bin", "dummy_file_70.bin",
        "dummy_file_71.bin", "dummy_file_72.bin", "dummy_file_73.bin", "dummy_file_74.bin", "dummy_file_75.bin",
        "dummy_file_76.bin", "dummy_file_77.bin", "dummy_file_78.bin", "dummy_file_79.bin", "dummy_file_80.bin",
        "dummy_file_81.bin", "dummy_file_82.bin", "dummy_file_83.bin", "dummy_file_84.bin", "dummy_file_85.bin",
        "dummy_file_86.bin", "dummy_file_87.bin", "dummy_file_88.bin", "dummy_file_89.bin", "dummy_file_90.bin",
        "dummy_file_91.bin", "dummy_file_92.bin", "dummy_file_93.bin", "dummy_file_94.bin", "dummy_file_95.bin",
        "dummy_file_96.bin", "dummy_file_97.bin", "dummy_file_98.bin", "dummy_file_99.bin", "dummy_file_100.bin",
        "dummy_file_101.bin", "dummy_file_102.bin", "dummy_file_103.bin", "dummy_file_104.bin", "dummy_file_105.bin",
        "dummy_file_106.bin", "dummy_file_107.bin", "dummy_file_108.bin", "dummy_file_109.bin", "dummy_file_110.bin",
        "dummy_file_111.bin", "dummy_file_112.bin", "dummy_file_113.bin", "dummy_file_114.bin", "dummy_file_115.bin",
        "dummy_file_116.bin", "dummy_file_117.bin", "dummy_file_118.bin", "dummy_file_119.bin", "dummy_file_120.bin",
        "dummy_file_121.bin", "dummy_file_122.bin", "dummy_file_123.bin", "dummy_file_124.bin", "dummy_file_125.bin",
        "dummy_file_126.bin", "dummy_file_127.bin", "dummy_file_128.bin", "dummy_file_129.bin", "dummy_file_130.bin",
        "dummy_file_131.bin", "dummy_file_132.bin", "dummy_file_133.bin", "dummy_file_134.bin", "dummy_file_135.bin",
        "dummy_file_136.bin", "dummy_file_137.bin", "dummy_file_138.bin", "dummy_file_139.bin", "dummy_file_140.bin",
        "dummy_file_141.bin", "dummy_file_142.bin", "dummy_file_143.bin", "dummy_file_144.bin", "dummy_file_145.bin",
        "dummy_file_146.bin", "dummy_file_147.bin", "dummy_file_148.bin", "dummy_file_149.bin", "dummy_file_150.bin",
        "dummy_file_151.bin", "dummy_file_152.bin", "dummy_file_153.bin", "dummy_file_154.bin", "dummy_file_155.bin",
        "dummy_file_156.bin", "dummy_file_157.bin", "dummy_file_158.bin", "dummy_file_159.bin", "dummy_file_160.bin",
        "dummy_file_161.bin", "dummy_file_162.bin", "dummy_file_163.bin", "dummy_file_164.bin", "dummy_file_165.bin",
        "dummy_file_166.bin", "dummy_file_167.bin", "dummy_file_168.bin", "dummy_file_169.bin", "dummy_file_170.bin",
        "dummy_file_171.bin", "dummy_file_172.bin", "dummy_file_173.bin", "dummy_file_174.bin", "dummy_file_175.bin",
        "dummy_file_176.bin", "dummy_file_177.bin", "dummy_file_178.bin", "dummy_file_179.bin", "dummy_file_180.bin",
        "dummy_file_181.bin", "dummy_file_182.bin", "dummy_file_183.bin", "dummy_file_184.bin", "dummy_file_185.bin",
        "dummy_file_186.bin", "dummy_file_187.bin", "dummy_file_188.bin", "dummy_file_189.bin", "dummy_file_190.bin",
        "dummy_file_191.bin", "dummy_file_192.bin", "dummy_file_193.bin", "dummy_file_194.bin", "dummy_file_195.bin",
        "dummy_file_196.bin", "dummy_file_197.bin", "dummy_file_198.bin", "dummy_file_199.bin", "dummy_file_200.bin",
        "dummy_file_201.bin", "dummy_file_202.bin", "dummy_file_203.bin", "dummy_file_204.bin", "dummy_file_205.bin",
        "dummy_file_206.bin", "dummy_file_207.bin", "dummy_file_208.bin", "dummy_file_209.bin", "dummy_file_210.bin",
        "dummy_file_211.bin", "dummy_file_212.bin", "dummy_file_213.bin", "dummy_file_214.bin", "dummy_file_215.bin",
        "dummy_file_216.bin", "dummy_file_217.bin", "dummy_file_218.bin", "dummy_file_219.bin", "dummy_file_220.bin",
        "dummy_file_221.bin", "dummy_file_222.bin", "dummy_file_223.bin", "dummy_file_224.bin", "dummy_file_225.bin",
        "dummy_file_226.bin", "dummy_file_227.bin", "dummy_file_228.bin", "dummy_file_229.bin", "dummy_file_230.bin",
        "dummy_file_231.bin", "dummy_file_232.bin", "dummy_file_233.bin", "dummy_file_234.bin", "dummy_file_235.bin",
        "dummy_file_236.bin", "dummy_file_237.bin", "dummy_file_238.bin", "dummy_file_239.bin", "dummy_file_240.bin",
        "dummy_file_241.bin", "dummy_file_242.bin", "dummy_file_243.bin", "dummy_file_244.bin", "dummy_file_245.bin",
        "dummy_file_246.bin", "dummy_file_247.bin", "dummy_file_248.bin", "dummy_file_249.bin", "dummy_file_250.bin",
        "dummy_file_251.bin", "dummy_file_252.bin", "dummy_file_253.bin", "dummy_file_254.bin", "dummy_file_255.bin",
        "dummy_file_256.bin", "dummy_file_257.bin", "dummy_file_258.bin", "dummy_file_259.bin", "dummy_file_260.bin",
        "dummy_file_261.bin", "dummy_file_262.bin", "dummy_file_263.bin", "dummy_file_264.bin", "dummy_file_265.bin",
        "dummy_file_266.bin", "dummy_file_267.bin", "dummy_file_268.bin", "dummy_file_269.bin", "dummy_file_270.bin",
        "dummy_file_271.bin", "dummy_file_272.bin", "dummy_file_273.bin", "dummy_file_274.bin", "dummy_file_275.bin",
        "dummy_file_276.bin", "dummy_file_277.bin", "dummy_file_278.bin", "dummy_file_279.bin", "dummy_file_280.bin",
        "dummy_file_281.bin", "dummy_file_282.bin", "dummy_file_283.bin", "dummy_file_284.bin", "dummy_file_285.bin",
        "dummy_file_286.bin", "dummy_file_287.bin", "dummy_file_288.bin", "dummy_file_289.bin", "dummy_file_290.bin",
        "dummy_file_291.bin", "dummy_file_292.bin", "dummy_file_293.bin", "dummy_file_294.bin", "dummy_file_295.bin",
        "dummy_file_296.bin", "dummy_file_297.bin", "dummy_file_298.bin", "dummy_file_299.bin", "dummy_file_300.bin",
        "dummy_file_301.bin", "dummy_file_302.bin", "dummy_file_303.bin", "dummy_file_304.bin", "dummy_file_305.bin",
        "dummy_file_306.bin", "dummy_file_307.bin", "dummy_file_308.bin", "dummy_file_309.bin", "dummy_file_310.bin",
        "dummy_file_311.bin", "dummy_file_312.bin", "dummy_file_313.bin", "dummy_file_314.bin", "dummy_file_315.bin",
        "dummy_file_316.bin", "dummy_file_317.bin", "dummy_file_318.bin", "dummy_file_319.bin", "dummy_file_320.bin",
        "dummy_file_321.bin", "dummy_file_322.bin", "dummy_file_323.bin", "dummy_file_324.bin", "dummy_file_325.bin",
        "dummy_file_326.bin", "dummy_file_327.bin", "dummy_file_328.bin", "dummy_file_329.bin", "dummy_file_330.bin",
        "dummy_file_331.bin", "dummy_file_332.bin", "dummy_file_333.bin", "dummy_file_334.bin", "dummy_file_335.bin",
        "dummy_file_336.bin", "dummy_file_337.bin", "dummy_file_338.bin", "dummy_file_339.bin", "dummy_file_340.bin",
        "dummy_file_341.bin", "dummy_file_342.bin", "dummy_file_343.bin", "dummy_file_344.bin", "dummy_file_345.bin",
        "dummy_file_346.bin", "dummy_file_347.bin", "dummy_file_348.bin", "dummy_file_349.bin", "dummy_file_350.bin",
        "dummy_file_351.bin", "dummy_file_352.bin", "dummy_file_353.bin", "dummy_file_354.bin", "dummy_file_355.bin",
        "dummy_file_356.bin", "dummy_file_357.bin", "dummy_file_358.bin", "dummy_file_359.bin", "dummy_file_360.bin",
        "dummy_file_361.bin", "dummy_file_362.bin", "dummy_file_363.bin", "dummy_file_364.bin", "dummy_file_365.bin",
        "dummy_file_366.bin", "dummy_file_367.bin", "dummy_file_368.bin", "dummy_file_369.bin", "dummy_file_370.bin",
        "dummy_file_371.bin", "dummy_file_372.bin", "dummy_file_373.bin", "dummy_file_374.bin", "dummy_file_375.bin",
        "dummy_file_376.bin", "dummy_file_377.bin", "dummy_file_378.bin", "dummy_file_379.bin", "dummy_file_380.bin",
        "dummy_file_381.bin", "dummy_file_382.bin", "dummy_file_383.bin", "dummy_file_384.bin", "dummy_file_385.bin",
        "dummy_file_386.bin", "dummy_file_387.bin", "dummy_file_388.bin", "dummy_file_389.bin", "dummy_file_390.bin",
        "dummy_file_391.bin", "dummy_file_392.bin", "dummy_file_393.bin", "dummy_file_394.bin", "dummy_file_395.bin",
        "dummy_file_396.bin", "dummy_file_397.bin", "dummy_file_398.bin", "dummy_file_399.bin", "dummy_file_400.bin",
};

typedef struct {
    int thread_id;
    int start_idx;
    int end_idx;
} thread_data_t;

// Function that each thread will execute
void* process_files(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    printf("Thread %d processing files from %d to %d\n", data->thread_id, data->start_idx, data->end_idx);

    for (int i = data->start_idx; i <= data->end_idx; i++) {
        printf("Thread %d processing file: %s\n", data->thread_id, file_names[i]);
        client(file_names[i]);
    }

    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];
    thread_data_t thread_data[NUM_THREADS];
    int rc;
    int files_per_thread = NUM_FILES / NUM_THREADS;
    int remainder = NUM_FILES % NUM_THREADS;
    int start_idx = 0;

    for (int t = 0; t < NUM_THREADS; t++) {
        int end_idx = start_idx + files_per_thread - 1;
        if (t == NUM_THREADS - 1) {
            end_idx += remainder; // Last thread handles any remaining files
        }

        thread_data[t].thread_id = t;
        thread_data[t].start_idx = start_idx;
        thread_data[t].end_idx = end_idx;

        printf("Creating thread %d to process files from %d to %d\n", t, start_idx, end_idx);

        rc = pthread_create(&threads[t], NULL, process_files, (void*)&thread_data[t]);
        if (rc) {
            printf("Error: Unable to create thread, %d\n", rc);
            return -1;
        }

        start_idx = end_idx + 1;
    }

    // Wait for all threads to complete
    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    printf("All threads completed.\n");
    return 0;

}
