/******************************************************************************
 * Spine Runtimes Software License
 * Version 2.3
 * 
 * Copyright (c) 2013-2015, Esoteric Software
 * All rights reserved.
 * 
 * You are granted a perpetual, non-exclusive, non-sublicensable and
 * non-transferable license to use, install, execute and perform the Spine
 * Runtimes Software (the "Software") and derivative works solely for personal
 * or internal use. Without the written permission of Esoteric Software (see
 * Section 2 of the Spine Software License Agreement), you may not (a) modify,
 * translate, adapt or otherwise create derivative works, improvements of the
 * Software or develop new applications using the Software or (b) remove,
 * delete, alter or obscure any trademarks or any copyright, trademark, patent
 * or other intellectual property or proprietary rights notices on or in the
 * Software, including any copy thereof. Redistributions in binary or source
 * form must include this license and terms.
 * 
 * THIS SOFTWARE IS PROVIDED BY ESOTERIC SOFTWARE "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL ESOTERIC SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include <spine/SkeletonBatch.h>
#include <spine/extension.h>
#include <algorithm>

USING_NS_CC;

namespace spine {

static SkeletonBatch* instance = nullptr;

void SkeletonBatch::setBufferSize (int vertexCount) {
	if (instance) delete instance;
	instance = new SkeletonBatch(vertexCount);
}

SkeletonBatch* SkeletonBatch::getInstance () {
	if (!instance) instance = new SkeletonBatch(8192);
	return instance;
}

SkeletonBatch::SkeletonBatch (int capacity) :
	_capacity(capacity), _position(0)
{
	_buffer = new V3F_C4B_T2F[capacity];
	_firstCommand = new Command();
	_command = _firstCommand;

	Director::getInstance()->getScheduler()->scheduleUpdate(this, -1, false);
}

SkeletonBatch::~SkeletonBatch () {
	Director::getInstance()->getScheduler()->unscheduleUpdate(this);

	Command* command = _firstCommand;
	while (command) {
		Command* next = command->_next;
		delete command;
		command = next;
	}

	delete [] _buffer;
}

void SkeletonBatch::update (float delta) {
	_position = 0;
	_command = _firstCommand;
}

void SkeletonBatch::addCommand (cocos2d::Renderer* renderer, float globalZOrder, GLuint textureID, GLProgramState* glProgramState,
	BlendFunc blendFunc, const TrianglesCommand::Triangles& triangles, const Mat4& transform, uint32_t transformFlags
) {
	CCASSERT(_position + triangles.vertCount < _capacity, "SkeletonBatch capacity is too small");

	memcpy(_buffer + _position, triangles.verts, sizeof(V3F_C4B_T2F) * triangles.vertCount);
	_command->_triangles->verts = _buffer + _position;
	_position += triangles.vertCount;

	_command->_triangles->vertCount = triangles.vertCount;
	_command->_triangles->indexCount = triangles.indexCount;
	_command->_triangles->indices = triangles.indices;

	_command->_trianglesCommand->init(globalZOrder, textureID, glProgramState, blendFunc, *_command->_triangles, transform, transformFlags);
	renderer->addCommand(_command->_trianglesCommand);

	if (!_command->_next) _command->_next = new Command();
	_command = _command->_next;
}

SkeletonBatch::Command::Command () :
	_next(nullptr)
{
	_trianglesCommand = new TrianglesCommand();
	_triangles = new TrianglesCommand::Triangles();
}

SkeletonBatch::Command::~Command () {
	delete _triangles;
	delete _trianglesCommand;
}

}
