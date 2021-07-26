// Copyright (c) 2014-2020 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#pragma once

#include "CoreMinimal.h"
#include "ISourceControlState.h"
#include "ISourceControlRevision.h"
#include "GitSourceControlRevision.h"

/** A consolidation of state priorities. */
namespace EGitState
{
	enum Type
	{
		NotAtHead,
		AddedAtHead,
		DeletedAtHead,
		LockedOther,
		NotLatest,
		/** Unmerged state (modified, but conflicts) */
		Unmerged,
		Added,
		Deleted,
		Modified,
		/** Not modified, but locked explicitly. */
		CheckedOut,
		Untracked,
		Lockable,
		Unmodified,
		Ignored,
		/** Whatever else. */
		None,
	};
}

/** Corresponds to diff file states. */
namespace EFileState
{
	enum Type
	{
		Unknown,
		Added,
		Copied,
		Deleted,
		Modified,
		Renamed,
		Missing,
		Unmerged,
	};
}

/** Where in the world is this file? */
namespace ETreeState
{
	enum Type
	{
		/** This file is synced to commit */
		Unmodified,
		/** This file is modified, but not in staging tree */
		Working,
		/** This file is in staging tree (git add) */
		Staged,
		/** This file is not tracked in the repo yet */
		Untracked,
		/** This file is ignored by the repo */
		Ignored,
		/** This file is outside the repo folder */
		NotInRepo,
	};
}

/** What is this file doing at HEAD? */
namespace ERemoteState
{
	enum Type
	{
		/** Local version is behind remote */
		NotAtHead,
		/** Remote file does not exist on local */
		AddedAtHead,
		/** Local was deleted on remote */
		DeletedAtHead,
		/** Not at the latest revision amongst the tracked branches */
		NotLatest,
		/** We want to branch off and ignore tracked branches */
		Branched,
	};
}

/** LFS locks status of this file */
namespace ELockState
{
	enum Type
	{
		Unknown,
		Unlockable,
		NotLocked,
		Locked,
		LockedOther,
	};
}

/** Combined state, for updating cache in a map. */
struct FGitState
{
	EFileState::Type FileState;
	ETreeState::Type TreeState;
	ERemoteState::Type RemoteState;
	ELockState::Type LockState;

	FGitState()
		: FileState(EFileState::Unknown)
		, TreeState(ETreeState::NotInRepo)
		, RemoteState(ERemoteState::Branched)
		, LockState(ELockState::Unknown)
	{
	}
};

class FGitSourceControlState : public ISourceControlState, public TSharedFromThis<FGitSourceControlState, ESPMode::ThreadSafe>
{
public:
	FGitSourceControlState( const FString& InLocalFilename)
		: LocalFilename(InLocalFilename)
		, TimeStamp(0)
	{
	}

	/** ISourceControlState interface */
	virtual int32 GetHistorySize() const override;
	virtual TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> GetHistoryItem(int32 HistoryIndex) const override;
	virtual TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> FindHistoryRevision(int32 RevisionNumber) const override;
	virtual TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> FindHistoryRevision(const FString& InRevision) const override;
	virtual TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> GetBaseRevForMerge() const override;
	virtual FName GetIconName() const override;
	virtual FName GetSmallIconName() const override;
	virtual FText GetDisplayName() const override;
	virtual FText GetDisplayTooltip() const override;
	virtual const FString& GetFilename() const override;
	virtual const FDateTime& GetTimeStamp() const override;
	virtual bool CanCheckIn() const override;
	virtual bool CanCheckout() const override;
	virtual bool IsCheckedOut() const override;
	virtual bool IsCheckedOutOther(FString* Who = NULL) const override;
	virtual bool IsCheckedOutInOtherBranch(const FString& CurrentBranch = FString()) const override;
	virtual bool IsModifiedInOtherBranch(const FString& CurrentBranch = FString()) const override;
	virtual bool IsCheckedOutOrModifiedInOtherBranch(const FString& CurrentBranch = FString()) const override { return IsCheckedOutInOtherBranch(CurrentBranch) || IsModifiedInOtherBranch(CurrentBranch); }
	virtual TArray<FString> GetCheckedOutBranches() const override { return TArray<FString>(); }
	virtual FString GetOtherUserBranchCheckedOuts() const override { return FString(); }
	virtual bool GetOtherBranchHeadModification(FString& HeadBranchOut, FString& ActionOut, int32& HeadChangeListOut) const override;
	virtual bool IsCurrent() const override;
	virtual bool IsSourceControlled() const override;
	virtual bool IsAdded() const override;
	virtual bool IsDeleted() const override;
	virtual bool IsIgnored() const override;
	virtual bool CanEdit() const override;
	virtual bool IsUnknown() const override;
	virtual bool IsModified() const override;
	virtual bool CanAdd() const override;
	virtual bool CanDelete() const override;
	virtual bool IsConflicted() const override;
	virtual bool CanRevert() const override;

private:
	EGitState::Type GetGitState() const;

public:
	/** History of the item, if any */
	TGitSourceControlHistory History;

	/** Filename on disk */
	FString LocalFilename;

	/** File Id with which our local revision diverged from the remote revision */
	FString PendingMergeBaseFileHash;

	FGitState State;

	/** Name of user who has locked the file */
	FString LockUser;

	/** The timestamp of the last update */
	FDateTime TimeStamp;

	/** The branch with the latest commit for this file */
	FString HeadBranch;

	/** The action within the head branch */
	FString HeadAction;

	/** The last file modification time in the head branch */
	int64 HeadModTime;

	/** The change list the last modification */
	FString HeadCommit;
};
