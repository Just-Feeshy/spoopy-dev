#include <spoopy_uniform.h>
#include <spoopy_log.h>

static size_t spoopy_uniform_next_pow2(size_t value) {
	size_t result = 1;

	while(result < value) {
		result <<= 1;
	}

	return result;
}

spoopy_hash_t spoopy_uniform_hash_string(const char* key) {
	uint32_t hash = 2166136261u;

	if(!key) {
		return 0;
	}

	for(const unsigned char* p = (const unsigned char*)key; *p; ++p) {
		hash ^= *p;
		hash *= 16777619u;
	}

	return hash;
}

size_t spoopy_uniform_ht_capacity(size_t item_count) {
	const size_t min_capacity = spoopy_max((size_t)8, item_count ? item_count * 2 : (size_t)8);
	return spoopy_uniform_next_pow2(min_capacity);
}

bool spoopy_uniform_ht_init(spoopy_uniform_ht_t* table, spoopy_mem_arena_t* arena, size_t item_count) {
	if(!table || !arena) {
		return false;
	}

	table->count = 0;
	table->capacity = spoopy_uniform_ht_capacity(item_count);
	table->entries = spoopy_arena_alloc(arena, table->capacity * sizeof(*table->entries));
	if(!table->entries) {
		table->capacity = 0;
		return false;
	}

	memset(table->entries, 0, table->capacity * sizeof(*table->entries));
	return true;
}

spoopy_uniform_t* spoopy_uniform_ht_get(spoopy_uniform_ht_t* table, const char* key) {
	if(!table || !table->entries || !key || table->capacity == 0) {
		return NULL;
	}

	const spoopy_hash_t hash = spoopy_uniform_hash_string(key);
	const size_t mask = table->capacity - 1;
	size_t idx = hash & mask;

	for(size_t probes = 0; probes < table->capacity; ++probes) {
		spoopy_uniform_ht_entry_t* entry = &table->entries[idx];

		if(!entry->key) {
			return NULL;
		}

		if(entry->hash == hash && strcmp(entry->key, key) == 0) {
			return &entry->value;
		}

		idx = (idx + 1) & mask;
	}

	return NULL;
}

bool spoopy_uniform_ht_insert_copy(
	spoopy_uniform_ht_t* table,
	spoopy_mem_arena_t* arena,
	const spoopy_uniform_t* src,
	uint8_t* buffer_data
) {
	if(!table || !table->entries || !arena || !src || !src->name) {
		return false;
	}

	const spoopy_hash_t hash = spoopy_uniform_hash_string(src->name);
	const size_t mask = table->capacity - 1;
	size_t idx = hash & mask;

	for(size_t probes = 0; probes < table->capacity; ++probes) {
		spoopy_uniform_ht_entry_t* entry = &table->entries[idx];

		if(!entry->key) {
			const char* key = spoopy_arena_strdup(arena, src->name);
			if(!key) {
				return false;
			}

			entry->key = key;
			entry->hash = hash;
			entry->value = *src;
			entry->value.name = key;
			entry->value.hash = hash;

			if(entry->value.type != SPOOPY_UNIFORM_SAMPLER_2D &&
			   entry->value.type != SPOOPY_UNIFORM_SAMPLER_CUBE) {
				entry->value.buffer_backed.data = buffer_data;
			}

			++table->count;
			return true;
		}

		if(entry->hash == hash && strcmp(entry->key, src->name) == 0) {
			SPOOPY_LOG_WARN("Duplicate uniform '%s' ignored", src->name);
			return true;
		}

		idx = (idx + 1) & mask;
	}

	SPOOPY_LOG_ERROR("Uniform hashtable is full; failed to insert '%s'", src->name);
	return false;
}
